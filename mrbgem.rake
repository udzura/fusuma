pjroot = File.expand_path('..', __FILE__)

MRuby::Gem::Specification.new('fusuma') do |spec|
  spec.license = 'GPL-2.0'
  spec.author  = 'Uchio Kondo'
  spec.summary = 'fusuma'
  spec.bins    = ['fusuma']

  spec.add_dependency 'mruby-print', core: 'mruby-print'
  spec.add_dependency 'mruby-string-ext', core: 'mruby-string-ext'
  spec.add_dependency 'mruby-io',    mgem: 'mruby-io'
  spec.add_dependency 'mruby-eval',  core: 'mruby-eval'
  spec.add_dependency 'mruby-curl',  mgem: 'mruby-curl'
  spec.add_dependency 'mruby-env',   mgem: 'mruby-env'
  #spec.add_dependency 'mruby-mtest',  mgem: 'mruby-mtest'

  spec.add_dependency 'mruby-fuse', path: "#{pjroot}/mrbgems/mruby-fuse"
end
