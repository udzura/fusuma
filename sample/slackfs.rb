FileStat = Struct.new(:st_mode, :st_nlink, :st_size)

class SlackFS
  def initialize(path, hook_url)
    @path = path
    @hook_url = hook_url
  end

  def on_getattr
    case @path
    when "/"
      return FileStat.new(FUSE::S_IFDIR|0755, 2, nil)
    when "/notify"
      return FileStat.new(FUSE::S_IFREG|0222, 1, 0)
    else
      return nil
    end
  end

  def on_open
    @path != "/slack" ? 0 : nil
  end

  def on_readdir
    return nil if @path != "/"
    return ["notify"]
  end

  def on_write(message, offset)
    req = HTTP::Request.new
    req.method = "POST"
    req.body = <<EOJ
payload={"text":#{message.inspect}}
EOJ

    req.headers['Content-Type'] = "application/x-www-form-urlencoded"
    res = Curl.new.send(@hook_url, req)
    puts "Post: #{res.body} / #{res.status_code}"

    return message.size
  end
end

FUSE.program_name = "fusuma"
FUSE.path = "/dev/slack"
FUSE.fsname = "slackfs"
FUSE.subtype = "slackfs"
FUSE.uid = FUSE.gid = 1000

FUSE.run SlackFS, ENV["SLACK_INCOMING_HOOK"]
