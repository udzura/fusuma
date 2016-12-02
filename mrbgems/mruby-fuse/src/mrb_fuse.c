/*
** mrb_fuse.c - FUSE class
**
** Copyright (c) Uchio Kondo 2016
**
** See Copyright Notice in LICENSE
*/

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/error.h>
#include "mrb_fuse.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

static int gettid(void)
{
  return (int)syscall(SYS_gettid);
}

static mrb_state *mrb_fuse_get_mrb_from_context(struct fuse_context *ctx)
{
  if(!ctx) {
    perror("fuse_get_context");
    exit(2);
  }
  return (mrb_state *)ctx->private_data;
}

static mrb_value mrb_fuse_find_or_create_instance(mrb_state *mrb, const char *path)
{
  struct RClass *fuse;
  mrb_value instance;
  fuse = mrb_module_get(mrb, "FUSE");
  instance = mrb_funcall(mrb, mrb_obj_value(fuse), "find_or_create_instance_by_path", 1, mrb_str_new_cstr(mrb, path));
  return instance;
}

static int mrb_fuse_getattr(const char *path, struct stat *stbuf)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value stat, st_size;
  int res = 0;
  printf("Call getattr for %s - %d\n", path, gettid());

  stat = mrb_funcall(mrb, instance, "on_getattr", 0);

  if(mrb_nil_p(stat)) {
    res = -ENOENT;
  } else {
    memset(stbuf, 0, sizeof(struct stat));

    stbuf->st_mode  = (mode_t) mrb_fixnum(mrb_funcall(mrb, stat, "st_mode", 0));
    stbuf->st_nlink = (nlink_t)mrb_fixnum(mrb_funcall(mrb, stat, "st_nlink", 0));
    st_size = mrb_funcall(mrb, stat, "st_size", 0);
    if(!mrb_nil_p(st_size)) {
      stbuf->st_size = (off_t)mrb_fixnum(st_size);
    }
  }

  return res;
}

static int mrb_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
       off_t offset, struct fuse_file_info *fi)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value files;
  (void) offset;
  (void) fi;
  printf("Call readdir for %s - %d\n", path, gettid());

  files = mrb_funcall(mrb, instance, "on_readdir", 0);

  if(mrb_nil_p(files)) {
    return -ENOENT;
  }

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  int len = RARRAY_LEN(files);
  for (int i = 0; i < len; i++) {
    filler(buf, RSTRING_PTR(mrb_ary_ref(mrb, files, i)), NULL, 0);
  }

  return 0;
}

static int mrb_fuse_truncate(const char *path, off_t size)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value ret;

  printf("Call truncate for %s - %d\n", path, gettid());

  if(! mrb_respond_to(mrb, instance, mrb_intern_cstr(mrb, "on_truncate"))){
    return 0;
  }
  ret = mrb_funcall(mrb, instance, "on_truncate", 1, mrb_fixnum_value((mrb_int)size));

  return mrb_fixnum(ret);
}

static int mrb_fuse_open(const char *path, struct fuse_file_info *fi)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value ret;
  (void) fi;
  printf("Call open for %s - %d\n", path, gettid());

  // TODO: use fuse_file_info in mruby layer
  /* if ((fi->flags & 3) != O_RDONLY) */
  /*   return -EACCES; */

  ret = mrb_funcall(mrb, instance, "on_open", 0);

  if(mrb_nil_p(ret)) {
    return -ENOENT;
  }
  return mrb_fixnum(ret);
}

static int mrb_fuse_read(const char *path, char *buf, size_t size, off_t offset,
                         struct fuse_file_info *fi)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value values;
  char *value;
  size_t len = 0;
  bool read_all = false;
  (void) fi;
  printf("Call read for %s - %d\n", path, gettid());

  mrb_bool has_read_all = mrb_respond_to(mrb, instance, mrb_intern_cstr(mrb, "on_read_all"));
  if (has_read_all) {
    values = mrb_funcall(mrb, instance, "on_read_all", 0);
    read_all = true;
    value = RSTRING_PTR(mrb_ary_ref(mrb, values, 0));
    len = mrb_fixnum(mrb_ary_ref(mrb, values, 1));
  } else {
    values = mrb_funcall(mrb, instance, "on_read", 2,
                         mrb_fixnum_value(size), mrb_fixnum_value(offset));
    value = RSTRING_PTR(mrb_ary_ref(mrb, values, 0));
    size = mrb_fixnum(mrb_ary_ref(mrb, values, 1));
  }

  if(mrb_nil_p(values))
    return -ENOENT;

  if (read_all) {
    if (offset < len) {
      if (offset + size > len)
        size = len - offset;
      memcpy(buf, value + offset, size);
    } else {
      size = 0;
    }
  } else {
    memcpy(buf, value, size);
  }

  return size;
}

static int mrb_fuse_write(const char *path, const char *buf, size_t size, off_t offset,
                          struct fuse_file_info *fi)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value value;
  char strbuf[size + 1];

  printf("Call write for %s - %d\n", path, gettid());

  if(memcpy(strbuf, buf, size) < 0) {
    mrb_sys_fail(mrb, "memcpy failed on_write");
  }
  strbuf[size] = '\0';

  value = mrb_funcall(mrb, instance, "on_write", 2,
                      mrb_str_new_cstr(mrb, strbuf), mrb_fixnum_value(offset));
  size = (size_t)mrb_fixnum(value);

  return size;
}

static int mrb_fuse_statfs(const char *path, struct statvfs *stbuf)
{
  /* FIXME: add dummy statfs for now */
  printf("Call statfs for %s - %d\n", path, gettid());

  return 0;
}

static int mrb_fuse_utimens(const char *path, const struct timespec ts[2])
{
  /* use ts[0](atime) and ts[1](mtime) for something, with conversion to mruby-time... */
  printf("Call utimens for %s - %d\n", path, gettid());

  return 0;
}

static int mrb_fuse_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
  /* FIXME: add dummy fsync for now */
  printf("Call fsync for %s - %d\n", path, gettid());

  return 0;
}

static int mrb_fuse_fallocate(const char *path, int mode, off_t offset, off_t length,
                              struct fuse_file_info *fi)
{
  /* FIXME: add dummy fallocate for now */
  printf("Call fallocate for %s - %d\n", path, gettid());

  return 0;
}

static int mrb_fuse_release(const char *path, struct fuse_file_info *fi)
{
  mrb_state *mrb = mrb_fuse_get_mrb_from_context(fuse_get_context());
  mrb_value instance = mrb_fuse_find_or_create_instance(mrb, path);
  mrb_value value;

  printf("Call release for %s - %d\n", path, gettid());

  /* Guard when there's no on_release implemented */
  if(! mrb_respond_to(mrb, instance, mrb_intern_cstr(mrb, "on_release"))){
    return 0;
  }

  value = mrb_funcall(mrb, instance, "on_release", 0);
  if(mrb_fixnum_p(value)) {
    return mrb_fixnum(value);
  }

  return 0;
}

static struct fuse_operations mrb_fuse_oper = {
  .getattr	= mrb_fuse_getattr,
  .readdir	= mrb_fuse_readdir,
  .truncate = mrb_fuse_truncate,
  .open		= mrb_fuse_open,
  .read		= mrb_fuse_read,
  .write  = mrb_fuse_write,
  .statfs = mrb_fuse_statfs,
  .fsync  = mrb_fuse_fsync,
  .utimens = mrb_fuse_utimens,
  .fallocate = mrb_fuse_fallocate,
  .release = mrb_fuse_release,
};

static mrb_value mrb_fuse_main(mrb_state *mrb, mrb_value self)
{
  mrb_int argc;
  char **argv;
  mrb_value *argv_;
  mrb_value str;
  int i, ai;

  mrb_get_args(mrb, "a", &argv_, &argc);
  argv = (char **)mrb_malloc(mrb, sizeof(char *) * (argc + 1));

  ai = mrb_gc_arena_save(mrb);
  for(i = 0; i < (int)argc; i++) {
    str = mrb_convert_type(mrb, argv_[i], MRB_TT_STRING, "String", "to_str");
    *argv = (char *)mrb_string_value_cstr(mrb, &str);
    argv++;
  }
  *argv = NULL;
  argv -= i;
  mrb_gc_arena_restore(mrb, ai);

  int r = fuse_main(argc, argv, &mrb_fuse_oper, (void *)mrb);
  return mrb_fixnum_value(r);
}

void mrb_mruby_fuse_gem_init(mrb_state *mrb)
{
  struct RClass *fuse;
  fuse = mrb_define_module(mrb, "FUSE");
  mrb_define_class_method(mrb, fuse, "invoke_fuse_main", mrb_fuse_main, MRB_ARGS_REQ(1));
  DONE;
}

void mrb_mruby_fuse_gem_final(mrb_state *mrb)
{
}
