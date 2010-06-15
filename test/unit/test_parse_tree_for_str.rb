require 'test/unit'
require_relative '../../ext/parse_tree'

class TestParseTree < Test::Unit::TestCase
  def setup
    @processor = ParseTree19.new(false)
  end

  def test_parse_tree_for_str
    # The order of the ops below is the order they appear in
    # add_to_parse_tree.inc/node.c
    # Note: if you ever wanted have a Ruby expression that causes
    # a given parse node to be created, see below.
    [
     # block:
     ['foo; bar',      [:scope, [:block, [:vcall, :foo], 
                                 [:block, [:vcall, :bar]]]]],

     # if:
     ['true if nil',   [:scope, [:if, [:nil], [:true]]]],
     ['if nil then true else false end', 
      [:scope, [:if, [:nil], [:true], [:false]]]],

     # Need case, when, opt_n

     ['5 while false', [:scope, [:while, true, [:false], [:lit, 5]]]],
     ['4 until true',  [:scope, [:until, true, [:true], [:lit, 4]]]],
     
     # iter:
     ['x { 5 }', 
      [:scope, [:iter, [:fcall, :x], [:scope, [:lit, 5]]]]],

     # Need for
       
     ['break',         [:scope, [:break]]],
     ['break 1',       [:scope, [:break, [:lit, 1]]]],
     ['next',          [:scope, [:next]]],
     ['next 1',        [:scope, [:next,  [:lit, 1]]]],
     ['return',        [:scope, [:return]]],
     ['return 4',      [:scope, [:return, [:lit, 4]]]],

     ['redo',          [:scope, [:redo]]],
     ['retry',         [:scope, [:retry]]],

     # begin
     ['begin; 1 ; end', 
      [:scope, [:block, [:begin], [:block, [:lit, 1]]]]],

     # rescue and resbody
     ['begin; 1 ; rescue; 2; else 3; end',
      [:scope, [:rescue, [:block, [:begin], [:block, [:lit, 1]]], 
                [:resbody, [:lit, 2]], [:lit, 3]]]],

     # ensure
     ['begin; 1 ; ensure; 2; end',
      [:scope, [:ensure, [:block, [:begin], [:block, [:lit, 1]]], 
                [:block, [:begin], [:block, [:lit, 2]]]]]],

     # and
     ['true and nil',  [:scope, [:and, [:true], [:nil]]]],
     # or
     ['true or false', [:scope, [:or,  [:true], [:false]]]],

     # lasgn
     ['x = 5',         [:scope, [:lasgn,  :x,   [:lit, 5]]]],
     # dasgn:
     ['x = nil; 1.times { x = 5 }',
      [:scope, [:block, [:lasgn, :x, [:nil]],
                [:block, [:iter, [:call, :times, [:lit, 1]], 
                          [:scope, [:dasgn, :x, [:lit, 5]]]]]]]],
     # dasgn_cur:
     ['1.times { x = 5 }',
      [:scope, [:iter, [:call, :times, [:lit, 1]], 
                [:scope, [:dasgn_curr, :x, [:lit, 5]]]]]],

     ['@x = 5',      [:scope, [:iasgn,  :@x,  [:lit, 5]]]],
     ['@@x = 5',     [:scope, [:cvasgn, :@@x, [:lit, 5]]]],

     ['$x = 5',      [:scope, [:gasgn,  :$x,  [:lit, 5]]]],

     ['x &&= 1',     [:scope, [:op_asgn_and,  
                               [:lvar, :x], [:lasgn, :x, [:lit, 1]]]]],
     ['x ||= 1',     [:scope, [:op_asgn_or,   
                               [:lvar, :x], [:lasgn, :x, [:lit, 1]]]]],

     # call:
     ['x.foo',         [:scope, [:call, :foo, [:vcall, :x]]]],
       
     # For fcall, see iter above.

     ['foo',           [:scope, [:vcall, :foo]]],

     ['super',         [:scope, [:zsuper]]],

     ['yield',         [:scope, [:yield]]],

     # For lvar, see x ||= 1

     # dvar: 
     ['1.times { x = 1; x }',
      [:scope, [:iter, [:call, :times, [:lit, 1]], 
                [:scope, [:block, [:dasgn_curr, :x, [:lit, 1]], 
                          [:block, [:dvar, :x]]]]]]],

     ['@x',            [:scope, [:ivar,  :@x]]],
     ['X',             [:scope, [:const, :X]]],
     ['@@X',           [:scope, [:cvar,  :@@X]]],
     ['$x',            [:scope, [:gvar,  :$x]]],

     ["5",             [:scope, [:lit, 5]]],
     ["'5'",           [:scope, [:str, '5']]],
     ['%x(5)',         [:scope, [:xstr, '5']]],
     ['"#{5}"',        [:scope, [:dstr, nil, [:evstr, [:lit, 5]]]]],
     ['`#{5}`',        [:scope, [:dxstr, nil, [:evstr, [:lit, 5]]]]],
     ['/#{5}/',        [:scope, [:dregx, '', [:evstr, [:lit, 5]]]]],
     ['/#{5}/o',       [:scope, [:dregx_once, '', [:evstr, [:lit, 5]]]]],
     [':"#{5}"',       [:scope, [:dsym, nil, [:evstr, [:lit, 5]]]]],

     ['alias a b',     [:scope, [:alias, [:lit, :a], [:lit, :b]]]],
     ['alias $a $b',   [:scope, [:valias, :$a, :$b]]],

     ['undef foo',     [:scope, [:undef, [:lit, :foo]]]],

     # dot2
     ['1..5.each{ 5 }',
      [:scope, [:dot2, [:lit, 1], 
                [:iter, [:call, :each, [:lit, 5]], [:scope, [:lit, 5]]]]]],
     # dot3
     ['1...5.each{ 5 }',
      [:scope, [:dot3, [:lit, 1], 
                [:iter, [:call, :each, [:lit, 5]], [:scope, [:lit, 5]]]]]],

     # Need flip2 and flip3

     ['self',          [:scope, [:self]]],
     ['nil',           [:scope, [:nil]]],
     ['true',          [:scope, [:true]]],
     ['false',         [:scope, [:false]]],

     ['M::Foo',        [:scope, [:colon2, :Foo, [:const, :M]]]],
     ['::Foo',         [:scope, [:colon3, :Foo]]],
     ['defined?(5)',   [:scope, [:defined, [:lit, 5]]]],
    ].each do |test_str, expected|
      actual   = @processor.parse_tree_for_str test_str, '(string)', 1
      assert_equal expected, actual
    end
  end
end
