#!/usr/bin/env rake
# -*- Ruby -*-
require 'rubygems'

ROOT_DIR = File.dirname(__FILE__)
def gemspec
  @gemspec ||= eval(File.read('.gemspec'), binding, '.gemspec')
end

require 'rake/gempackagetask'
desc "Build the gem"
task :package=>:gem
task :gem=>:gemspec do
  Dir.chdir(ROOT_DIR) do
    sh "gem build .gemspec"
    FileUtils.mkdir_p 'pkg'
    FileUtils.mv "#{gemspec.name}-#{gemspec.version}.gem", 'pkg'
  end
end

desc "Install the gem locally"
task :install => :gem do
  Dir.chdir(ROOT_DIR) do
    sh %{gem install --local pkg/#{gemspec.name}-#{gemspec.version}}
  end
end

require 'rbconfig'
RUBY_PATH = File.join(Config::CONFIG['bindir'],  
                      Config::CONFIG['RUBY_INSTALL_NAME'])

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

desc 'Create a GNU-style ChangeLog via git2cl'
task :ChangeLog do
  system('git log --pretty --numstat --summary | git2cl > ChangeLog')
end

require 'rake/testtask'
desc 'Test units - the smaller tests'
Rake::TestTask.new(:'test:unit') do |t|
  t.libs << './ext'
  t.test_files = FileList['test/unit/**/*.rb']
  # t.pattern = 'test/**/*test-*.rb' # instead of above
  t.verbose = true
end
task :'test:unit' => [:ext]

desc "Test everything - same as test."

task :'check' do
  run_standalone_ruby_file(File.join(%W(#{ROOT_DIR} test unit)))
end

task :default => [:test]
task :test => [:'test:unit']

desc "Generate the gemspec"
task :generate do
  puts gemspec.to_ruby
end

desc "Validate the gemspec"
task :gemspec do
  gemspec.validate
end


# ---------  RDoc Documentation ------
require 'rake/rdoctask'
desc 'Generate rdoc documentation'
Rake::RDocTask.new('rdoc') do |rdoc|
  rdoc.rdoc_dir = 'doc'
  rdoc.title    = "ParseTree Documentation"
  # Show source inline with line numbers
  rdoc.options << '--inline-source' << '--line-numbers'
  # Make the readme file the start page for the generated html
  rdoc.options << '--main' << 'README.md'
  rdoc.rdoc_files.include(%w(ext/**/*.c README.md))
end
