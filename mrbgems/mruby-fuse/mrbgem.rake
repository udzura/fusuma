MRuby::Gem::Specification.new('mruby-fuse') do |spec|
  spec.license = 'GPL-2.0'
  spec.authors = 'Uchio Kondo'

  flags = `pkg-config --cflags fuse`.chomp.split(" ")
  spec.cc.flags.concat flags.select{|f| f.start_with? "-D" }
  spec.cc.include_paths << flags.last.sub("-I", "")

  libs = `pkg-config --libs fuse`.chomp.split(" ")

  spec.linker.flags.concat flags.select{|f| f.start_with? "-L" }
  spec.linker.libraries << 'osxfuse' << 'iconv'
end
