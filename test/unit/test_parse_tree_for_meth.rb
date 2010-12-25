require 'test/unit'
require_relative '../../ext/parse_tree'

def five; 5 end

class TestParseTree < Test::Unit::TestCase
  def setup
    @processor = ParseTree19.new(false)
  end

  def test_parse_tree_for_method

    def six
      x = 2
      y = 3
      return x * y
    end

    [
     [:five,      
      [:scope, [:args, 0, [:args_aux]], [:lit, 5]]],

     [:six, 
      [:scope, [:args, 0, [:args_aux]], 
       [:block, [:lasgn, :x, [:lit, 2]], 
        [:block, [:lasgn, :y, [:lit, 3]], 
         [:block, [:call, :*, [:lvar, :x], [:array, 1, [:lvar, :y]]]]]]]]
    ].each do |meth, expected|
      actual  = @processor.parse_tree_for_method(method(meth), true)
      assert_equal expected, actual
    end
  end
end
