pjroot = File.expand_path('..', __FILE__)

MRuby::Gem::Specification.new('fusuma') do |spec|
  spec.license = 'MIT'
  spec.author  = 'MRuby Developer'
  spec.summary = 'fusuma'
  spec.bins    = ['fusuma']

  spec.add_dependency 'mruby-print', :core => 'mruby-print'
  spec.add_dependency 'mruby-mtest', :mgem => 'mruby-mtest'

  spec.add_dependency 'mruby-fuse',  :path => "#{pjroot}/mrbgems/mruby-fuse"
end
