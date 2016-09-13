def gem_config(conf)
  #conf.gembox 'default'

  # be sure to include this gem (the cli app)
  conf.gem File.expand_path(File.dirname(__FILE__))
end

MRuby::Build.new do |conf|
  if `uname` =~ /linux/i
    toolchain :gcc
  else
    toolchain :clang
  end

  conf.enable_bintest
  conf.enable_debug
  conf.enable_test

  conf.gembox 'default'
  gem_config(conf)
end

# MRuby::Build.new('x86_64-pc-linux-gnu') do |conf|
#   toolchain :gcc

#   gem_config(conf)
# end
