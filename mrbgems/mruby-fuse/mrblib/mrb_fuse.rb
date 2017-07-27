module FUSE
  S_IFDIR = 0040000
  S_IFREG = 0100000
  S_IFLNK = 0120000

  class << self
    def pool
      @__pool__ ||= {}
    end
    attr_accessor :program_name, :path, :fsname, :subtype,
                  :uid, :gid, :allow_root,
                  :multithread,
                  :daemonize,
                  :extra_fuse_options
    attr_reader   :klass, :args

    def help
      invoke_fuse_main(['mruby-fuse', '/tmp', '-h'])
    end

    def run(klass, *args)
      @klass = klass
      @args = args
      fuse_args = []
      fuse_args << program_name
      fuse_args << path
      fuse_args << '-s'                         unless multithread
      fuse_args << '-o' << 'default_permissions'
      fuse_args << '-o' << "fsname=#{fsname}"   if fsname
      fuse_args << '-o' << "subtype=#{subtype}" if subtype
      fuse_args << '-o' << 'allow_root'         if allow_root
      fuse_args << '-o' << "uid=#{uid}"         if uid
      fuse_args << '-o' << "gid=#{gid}"         if gid
      if extra_fuse_options
        fuse_args.concat extra_fuse_options
      end
      fuse_args << '-f'                         unless daemonize
      invoke_fuse_main(fuse_args)
    end

    def find_or_create_instance_by_path(path)
      if i = pool[path]
        return i
      else
        i = klass.new(path, *args)
        pool[path] = i
        return i
      end
    end
  end
end
