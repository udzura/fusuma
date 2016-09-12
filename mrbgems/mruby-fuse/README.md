# mruby-fuse   [![Build Status](https://travis-ci.org/udzura/mruby-fuse.svg?branch=master)](https://travis-ci.org/udzura/mruby-fuse)
FUSE class
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'udzura/mruby-fuse'
end
```
## example
```ruby
p FUSE.hi
#=> "hi!!"
t = FUSE.new "hello"
p t.hello
#=> "hello"
p t.bye
#=> "hello bye"
```

## License
under the GPL-2.0 License:
- see LICENSE file
