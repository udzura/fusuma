FUSE.program_name = "fusuma"
FUSE.path = "/tmp/foo"
FUSE.fsname = "fusuma"
FUSE.subtype = "fusuma"
FUSE.uid = FUSE.gid = 1000

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
      return FileStat.new(FUSE::S_IFDIR|0755, 2, nil)
    when "/hello"
      return FileStat.new(FUSE::S_IFREG|0444, 1, @value.size)
    when "/world"
      return FileStat.new(FUSE::S_IFREG|0644, 1, @value.size)
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

  def on_truncate(size)
    @value = ""
    return 0
  end

  def on_write(buf, offset)
    @value << buf
    return buf.size
  end
end

FUSE.run Example
