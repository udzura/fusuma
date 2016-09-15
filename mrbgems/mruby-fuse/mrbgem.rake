MRuby::Gem::Specification.new('mruby-fuse') do |spec|
  spec.license = 'GPL-2.0'
  spec.authors = 'Uchio Kondo'

  flags = `pkg-config --cflags fuse`.chomp.split(" ")
  spec.cc.flags.concat flags.select{|f| f.start_with? "-D" }
  spec.cc.include_paths << flags.last.sub("-I", "")

  lib_flags = `pkg-config --libs fuse`.chomp.split(" ")
  libs = lib_flags.select{|f| f.start_with? "-l" }.map{|f| f.sub("-l", "") }

  spec.linker.flags.concat lib_flags.select{|f| f.start_with? "-L" }
  spec.linker.libraries.concat libs

  spec.add_dependency 'mruby-struct', core: 'mruby-struct'
end
