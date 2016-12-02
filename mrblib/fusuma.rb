def __main__(argv)
  if argv.size < 2 or argv[1].end_with?("help")
    puts <<EOH
Fusuma v#{Fusuma::VERSION}
Usage:
\t#{argv[0]} fuse-def.rb
EOH
  elsif argv[1].end_with?("version")
    puts "Fusuma v#{Fusuma::VERSION}"
  else
    eval(File.read argv[1])
  end
end
