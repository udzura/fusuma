def have_pkg_config?
  system "which pkg-config > /dev/null"
end

def get_cflags
  have_pkg_config? ? `pkg-config --cflags fuse` :
    "-D_FILE_OFFSET_BITS=64 -I/usr/include/fuse"
end

def get_libs
  have_pkg_config? ? `pkg-config --libs fuse` :
    "-lfuse -pthread"
end

MRuby::Gem::Specification.new('mruby-fuse') do |spec|
  spec.license = 'GPL-2.0'
  spec.authors = 'Uchio Kondo'

  flags = get_cflags.chomp.split(" ")
  spec.cc.flags.concat flags.select{|f| f.start_with? "-D" }
  spec.cc.include_paths << flags.last.sub("-I", "")

  lib_flags = get_libs.chomp.split(" ")
  libs = lib_flags.select{|f| f.start_with? "-l" }.map{|f| f.sub("-l", "") }

  spec.linker.flags.concat lib_flags.delete_if{|f| f.start_with? "-l" }
  spec.linker.libraries.concat libs

  spec.add_dependency 'mruby-struct', core: 'mruby-struct'
end
