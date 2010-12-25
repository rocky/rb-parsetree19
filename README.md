# rb-parsetree19

A C extension to support ParseTree in Ruby 1.9

## Requirements

A patched version of Ruby 1.9.2 is needed. In fact the same patched version
as needed by rb-threadframe (and thus trepanning).

Usage:

    require 'parse_tree'
    processor = ParseTree19.new(false)
    processor.parse_tree_for_str('x=1; y=2', '(string'), 1)
      => [:scope, [:block, [:lasgn, :x, [:lit, 1]], [:block, [:lasgn, :y, [:lit, 2]]]]]
    def five; 5 end
    processor.parse_tree_for_method(method(:five), true)
      => [:scope, [:args, 0, [:args_aux]], [:lit, 5]]
    puts processor.parse_tree_for_method(method(:five, false)
      => nil
    ###########################################################
    ## Do NOT use this node dump for any purpose other than  ##
    ## debug and research.  Compatibility is not guaranteed. ##
    ###########################################################
    
    # @ NODE_SCOPE (line: 6)
    # +- nd_tbl: (empty)
    # +- nd_args:
    # |   @ NODE_ARGS (line: 6)
    # |   +- nd_frml: 0
    # |   +- nd_next:
    # |   |   @ NODE_ARGS_AUX (line: 6)
    # |   |   +- nd_rest: (null)
    # |   |   +- nd_body: (null)
    # |   |   +- nd_next:
    # |   |       (null node)
    # |   +- nd_opt:
    # |       (null node)
    # +- nd_body:
    #     @ NODE_LIT (line: 6)
    #     +- nd_lit: 5
