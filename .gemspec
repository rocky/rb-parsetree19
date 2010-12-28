# -*- Ruby -*-
# -*- encoding: utf-8 -*-
require 'rake'
require 'rubygems' unless  Object.const_defined?(:Gem)

EXT_FILES     = FileList[%w(ext/*.c ext/*.h ext/*.inc)]
INCLUDE_FILES = FileList['include/*.h']
LIB_FILES     = FileList['lib/*.rb']
TEST_FILES    = FileList['test/*/*.rb']
COMMON_FILES  = FileList[%w(README.md Rakefile NEWS ChangeLog)]
ALL_FILES     = COMMON_FILES + INCLUDE_FILES + LIB_FILES + EXT_FILES + 
  TEST_FILES

ROOT_DIR ||= File.dirname(__FILE__)
PACKAGE_VERSION = open(File.join(ROOT_DIR, 'ext/parse_tree.c')) do |f| 
  f.grep(/^#define PARSETREE19_VERSION/).first[/"(.+)"/,1]
end

# Base GEM Specification
@spec = Gem::Specification.new do |spec|
  spec.name = "parsetree19"
  
  spec.homepage = "http://github.com/rocky/rb-parsetree19/tree/master"
  spec.summary = "Frame introspection"
  spec.description = <<-EOF

rb-parsetree is the C extension part of ParseTree for Ruby 1.9. A patched version of Ruby 1.9.2 is required.
EOF

  spec.version = PACKAGE_VERSION
  spec.require_path = 'lib'
  spec.extensions = ["ext/extconf.rb"]

  spec.author = "R. Bernstein"
  spec.email  = "rockyb@rubyforge.org"
  spec.platform = Gem::Platform::RUBY
  spec.files = ALL_FILES.to_a  

  spec.required_ruby_version = '~> 1.9.2frame'
  spec.date = Time.now
  # spec.rubyforge_project = 'rocky-hacks'
  
  # rdoc
  spec.has_rdoc = true
  spec.extra_rdoc_files = ['README.md'] + FileList['ext/*.c']
end
