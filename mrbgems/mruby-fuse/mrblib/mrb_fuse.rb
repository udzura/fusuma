module FUSE
  S_IFDIR = 0040000
  S_IFREG = 0100000

  class << self
    def pool
      @__pool__ ||= {}
    end
    attr_accessor :program_name, :path, :fsname, :subtype
    attr_reader   :klass, :args

    def run(klass, *args)
      @klass = klass
      @args = args
      fuse_args = []
      fuse_args << program_name
      fuse_args << path
      fuse_args << '-o' << 'default_permissions'
      fuse_args << '-o' << "fsname=#{fsname}"  if fsname
      fuse_args << '-o' << "fsname=#{subtype}" if subtype
      fuse_args << '-f'
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

  FileStat = Struct.new(:st_mode, :st_nlink, :st_size)

  class Example
    # This is called just before on_getattr
    def initialize(path, *a)
      @path = path
      case @path
      when "/hello"
        @value = "Hello, mruby fuse!!\n"
      when "/world"
        @value = "Hello, yet another mruby fuse!!\n"
      end
    end

    def on_getattr
      case @path
      when "/"
        return FileStat.new(S_IFDIR|0755, 2, nil)
      when "/hello", "/world"
        return FileStat.new(S_IFREG|0444, 1, @value.size)
      else
        return nil
      end
    end

    def on_open
      if ["/hello", "/world"].include? @path
        return 0
      else
        return nil
      end
    end

    def on_readdir
      return nil if @path != "/"
      return ["hello", "world"]
    end

    def on_read_all
      return nil if @path == "/"
      return [@value, @value.size]
    end
  end
end
