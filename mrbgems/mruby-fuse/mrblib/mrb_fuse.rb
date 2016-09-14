module FUSE
  class << self
    def instance
      @__instance__
    end
    attr_accessor :program_name, :path, :fsname, :subtype

    def run(klass, *args)
      @__instance__ = klass.new(*args)
      fuse_args = []
      fuse_args << program_name
      fuse_args << path
      fuse_args << '-o' << 'default_permissions'
      fuse_args << '-o' << "fsname=#{fsname}"  if fsname
      fuse_args << '-o' << "fsname=#{subtype}" if subtype
      invoke_fuse_main(fuse_args)
    end
  end

  class Example
    def read_all(path)
      case path
      when "/hello"
        ret = "Hello, mruby fuse!!\n"
        return [ret, ret.size]
      when "/world"
        ret = "Hello, yet another mruby fuse!!\n"
        return [ret, ret.size]
      else
        return nil
      end
    end
  end
end
