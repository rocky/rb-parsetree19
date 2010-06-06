#!/usr/bin/env rake
# -*- Ruby -*-
require 'rubygems'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'

rake_dir = File.dirname(__FILE__)

require 'rbconfig'
RUBY_PATH = File.join(Config::CONFIG['bindir'],  
                      Config::CONFIG['RUBY_INSTALL_NAME'])

SO_NAME = 'parsetree19.so'

PACKAGE_VERSION = open("ext/thread_frame.c") do |f| 
  f.grep(/^#define PARSETREE19_VERSION/).first[/"(.+)"/,1]
end

EXT_FILES    = FileList[%w(ext/*.c ext/*.h)]
LIB_FILES    = FileList['lib/*.rb']
TEST_FILES   = FileList['test/**/*.rb']
COMMON_FILES = FileList[%w(README.md Rakefile)]
ALL_FILES    = COMMON_FILES + LIB_FILES + EXT_FILES + TEST_FILES

desc 'Create the core thread-frame shared library extension'
task :ext do
  Dir.chdir('ext') do
    system("#{Gem.ruby} extconf.rb && make")
  end
end

desc 'Remove built files'
task :clean do
  cd 'ext' do
    if File.exist?('Makefile')
      sh 'make clean'
      rm 'Makefile'
    end
    derived_files = Dir.glob('.o') + Dir.glob('*.so')
    rm derived_files unless derived_files.empty?
  end
end

def run_standalone_ruby_file(directory)
  puts ('*' * 10) + ' ' + directory + ' ' + ('*' * 10)
  Dir.chdir(directory) do
    Dir.glob('*.rb').each do |ruby_file|
      puts ('-' * 20) + ' ' + ruby_file + ' ' + ('-' * 20)
      system(RUBY_PATH, ruby_file)
    end
  end
end

desc "Create a GNU-style ChangeLog via git2cl"
task :ChangeLog do
  system("git log --pretty --numstat --summary | git2cl > ChangeLog")
end

task :default => [:test]

desc 'Test units - the smaller tests'
task :'test:unit' => [:ext] do |t|
  Rake::TestTask.new(:'test:unit') do |t|
    t.libs << './ext'
    t.test_files = FileList['test/unit/**/*.rb']
    # t.pattern = 'test/**/*test-*.rb' # instead of above
    t.verbose = true
  end
end

desc 'Test everything - unit tests for now.'
task :test do
  exceptions = ['test:unit'].collect do |task|
    begin
      Rake::Task[task].invoke
      nil
    rescue => e
      e
    end
  end.compact
  
  exceptions.each {|e| puts e;puts e.backtrace }
  raise "Test failures" unless exceptions.empty?
end

desc "Test everything - same as test."

task :'check' do
  run_standalone_ruby_file(File.join(%W(#{rake_dir} test unit)))
end

# ---------  RDoc Documentation ------
desc 'Generate rdoc documentation'
Rake::RDocTask.new('rdoc') do |rdoc|
  rdoc.rdoc_dir = 'doc/rdoc'
  rdoc.title    = 'rb-threadframe'
  # Show source inline with line numbers
  rdoc.options << '--inline-source' << '--line-numbers'
  # Make the readme file the start page for the generated html
  rdoc.options << '--main' << 'README.md'
  rdoc.rdoc_files.include('ext/**/*.c',
                          'README.md')
end

# Base GEM Specification
spec = Gem::Specification.new do |spec|
  spec.name = "rb-threadframe"
  
  spec.homepage = "http://github.com/rocky/rb-parsetree19/tree/master"
  spec.summary = "Frame introspection"
  spec.description = <<-EOF

rb-threadframe gives introspection access for frames of a thread.
EOF

  spec.version = PACKAGE_VERSION
  spec.require_path = 'lib'
  spec.extensions = ["ext/extconf.rb"]

  spec.author = "R. Bernstein"
  spec.email  = "rockyb@rubyforge.org"
  spec.platform = Gem::Platform::RUBY
  spec.files = ALL_FILES.to_a  

  spec.required_ruby_version = '>= 1.9.1'
  spec.date = Time.now
  # spec.rubyforge_project = 'rocky-hacks'
  
  # rdoc
  spec.has_rdoc = true
  spec.extra_rdoc_files = ['README.md', 'threadframe.rd'] + 
                           FileList['ext/*.c']
end

# Rake task to build the default package
Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar = true
end

def install(spec, *opts)
  args = ['gem', 'install', "pkg/#{spec.name}-#{spec.version}.gem"] + opts
  system(*args)
end

desc 'Install locally'
task :install => :package do
  Dir.chdir(File::dirname(__FILE__)) do
    # ri and rdoc take lots of time
    install(spec, '--no-ri', '--no-rdoc')
  end
end    

task :install_full => :package do
  Dir.chdir(File::dirname(__FILE__)) do
    install(spec)
  end
end    