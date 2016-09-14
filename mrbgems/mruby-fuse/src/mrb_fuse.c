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

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include "mrb_fuse.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

static const char *hello_str = "Hello mruby!\n";
static const char *hello_path = "/hello";

static int mrb_fuse_getattr(const char *path, struct stat *stbuf)
{
  int res = 0;

  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else if (strcmp(path, hello_path) == 0) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(hello_str);
  } else {
    res = -ENOENT;
  }

  return res;
}

static int mrb_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
       off_t offset, struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;
  // (void) flags;

  if (strcmp(path, "/") != 0)
    return -ENOENT;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, hello_path + 1, NULL, 0);

  return 0;
}

static int mrb_fuse_open(const char *path, struct fuse_file_info *fi)
{
  if (strcmp(path, hello_path) != 0)
    return -ENOENT;

  if ((fi->flags & 3) != O_RDONLY)
    return -EACCES;

  return 0;
}

static int mrb_fuse_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
  mrb_state *mrb;
  struct RClass *fuse;
  mrb_value instance, values;
  char *value;
  size_t len;
  (void) fi;
  struct fuse_context *ctx = fuse_get_context();
  if(!ctx) {
    perror("fuse_get_context");
    exit(2);
  }

  mrb = (mrb_state *)ctx->private_data;
  fuse = mrb_module_get(mrb, "FUSE");
  instance = mrb_funcall(mrb, mrb_obj_value(fuse), "instance", 0);

  values = mrb_funcall(mrb, instance, "read_all", 1, mrb_str_new_cstr(mrb, path));

  /* values = mrb_funcall(mrb, instance, "read", 3, */
  /*                      mrb_str_new_cstr(path), mrb_fixnum_value(size), mrb_fixnum_value(offset)); */

  if(mrb_nil_p(values))
    return -ENOENT;

  value = RSTRING_PTR(mrb_ary_ref(mrb, values, 0));
  len = mrb_fixnum(mrb_ary_ref(mrb, values, 1));

  if (offset < len) {
    if (offset + size > len)
      size = len - offset;
    memcpy(buf, hello_str + offset, size);
  } else {
    size = 0;
  }

  return size;
}

static struct fuse_operations mrb_fuse_oper = {
  .getattr	= mrb_fuse_getattr,
  .readdir	= mrb_fuse_readdir,
  .open		= mrb_fuse_open,
  .read		= mrb_fuse_read,
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
