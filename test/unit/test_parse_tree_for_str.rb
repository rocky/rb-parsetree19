require 'test/unit'
require_relative '../../ext/parse_tree'

class TestParseTree < Test::Unit::TestCase
  def setup
    @processor = ParseTree19.new(false)
  end

  def test_parse_tree_for_str
    # The order of the ops below is the order they appear in
    # parse_tree.c/node.c
    [
     ['redo',        [:scope, [:redo]]],
     ['retry',       [:scope, [:retry]]],

     ['x = 5',       [:scope, [:lasgn,  :x,   [:lit, 5]]]],
     # dasgn: x = nil; 1.times { x = foo }
     # dasgn_cur: 1.times { x = foo }
     ['@x = 5',      [:scope, [:iasgn,  :@x,  [:lit, 5]]]],
     ['@@x = 5',     [:scope, [:cvasgn, :@@x, [:lit, 5]]]],

     ['yield',       [:scope, [:yield]]],

     # lvar: x 
     # dvar: 1.times { x = 1; x }
     ['@x',          [:scope, [:ivar,  :@x]]],
     ['X',           [:scope, [:const, :X]]],
     ['@@X',         [:scope, [:cvar,  :@@X]]],

     ["5",           [:scope, [:lit, 5]]],
     ["'5'",         [:scope, [:str, '5']]],
     ['%x(5)',       [:scope, [:xstr, '5']]],
     ['"#{5}"',      [:scope, [:dstr, nil, [:evstr, [:lit, 5]]]]],
     ['`#{5}`',      [:scope, [:dxstr, nil, [:evstr, [:lit, 5]]]]],
     ['/#{5}/',      [:scope, [:dregx, '', [:evstr, [:lit, 5]]]]],
     ['/#{5}/o',     [:scope, [:dregx_once, '', [:evstr, [:lit, 5]]]]],
     [':"#{5}"',     [:scope, [:dsym, nil, [:evstr, [:lit, 5]]]]],

     ['self',        [:scope, [:self]]],
     ['nil',         [:scope, [:nil]]],
     ['true',        [:scope, [:true]]],
     ['false',       [:scope, [:false]]],

     ['M::Foo',      [:scope, [:colon2, :Foo, [:const, :M]]]],
     ['::Foo',       [:scope, [:colon3, :Foo]]],
     ['defined?(5)', [:scope, [:defined, [:lit, 5]]]],
    ].each do |test_str, expected|
      actual   = @processor.parse_tree_for_str test_str, '(string)', 1
      assert_equal expected, actual
    end
  end
end
