# Fusuma

[![Build Status](https://travis-ci.org/udzura/fusuma.svg?branch=master)](https://travis-ci.org/udzura/fusuma)

FUSe Upon Mruby-script Assistance

## How to build

Install mruby build-ready environment. Then:

```
rake
# TODO: more useful fusuma command
sudo cp ./mruby/bin/mruby /usr/local/bin/fusuma
```

## Example

Just define a class that respond to `on_open`, `on_read`, `on_getattr`, ... as [defined in libfuse's strust fuse_operation](https://fossies.org/dox/fuse-2.9.7/structfuse__operations.html).

```ruby
# Duck typing object to return from on_getattr
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

  # If you want to read object with size/offset, you can define `on_read(size, offset)' instead
  def on_read_all
    return nil if @path == "/"
    return [@value, @value.size]
  end
end

FUSE.program_name = "fusuma"
FUSE.path = "/tmp/foo"
FUSE.fsname = "fusuma"
FUSE.subtype = "fusuma"

# Will be stuck to front
FUSE.run Example
```

Run the script and then you get:

```console
$ sudo ls -l /tmp/foo
total 0
-r--r--r-- 1 root root 20 Jan  1  1970 hello
-r--r--r-- 1 root root 32 Jan  1  1970 world
$ sudo cat /tmp/foo/hello
Hello, mruby fuse!!
$ sudo cat /tmp/foo/world
Hello, yet another mruby fuse!!
```

TODO: Fusuma now just supports `getattr/readdir/open/read` for hello world. PRs are welcome :)

## Development

* `rake compile` will create binaries.
* `rake` may not be passed unless you are not on Linux.
* This project is built upon great [mruby-cli](https://github.com/hone/mruby-cli). Please browse its README.

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/udzura/fusuma. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

Fusuma is under the GPL v-2.1 License (same as libfuse): See [LICENSE](./LICENSE) file.
