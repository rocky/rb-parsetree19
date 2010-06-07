require 'test/unit'
require_relative '../../ext/parse_tree'

class TestParseTree < Test::Unit::TestCase
  def setup
    @processor = ParseTree19.new(false)
  end

  def test_parse_tree_for_str
    [['nil',   [:scope, [:nil]]],
     ['true',  [:scope, [:true]]],
     ['false', [:scope, [:false]]],
     ['5',     [:scope, [:lit, 5]]],
     ["'5'",   [:scope, [:str, '5']]],
     ['%x(5)', [:scope, [:xstr, '5']]]
    ].each do |test_str, expected|
      actual   = @processor.parse_tree_for_str test_str, '(string)', 1
      assert_equal expected, actual
    end
  end
end
