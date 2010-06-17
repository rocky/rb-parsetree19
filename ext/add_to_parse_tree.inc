/* -*- C -*- */
/* Actions for building up a parse tree. Most of this parallels 
   dump_node() in Ruby 1.9's node.c. 
*/

/* node name as a C string. It will start with "NODE_",
   e.g. "NODE_NIL" */
#define NODE_NAME(node)				\
    ruby_node_name(nd_type(node))

/* The below is a merger of dump_node() of Ruby 1.9's node.c and
   add_to_parse_tree() of ParseTree's parsetree.rb

   As such, the macros below may seem weird because they are from node.c
   They try to do corresponding ParseTree actions.
*/

#define AR(str) str

#define A_INT(val) (val)
#define A_LIT(lit) AR(rb_inspect(lit))

#define ANN(ann)    /* Discard annotation */
#define LAST_NODE   /* Ignore adjustment of node-placement indentation. */

#define D_NULL_NODE wrap_into_node("NODE_NIL", Qnil)

/* Note: not completely sure if we should keep this, modifiy it slightly
   or use just the expansion inline. */
#define D_NODE_HEADER(node)						\
    current = rb_ary_new();						\
    node_name = pt_node_name(NODE_NAME(node));				\
    rb_ary_push(ary, current);						\
    rb_ary_push(current, node_name);

#define F_LIT(name, ann)			\
    rb_ary_push(current, node->name)

#define F_ID(name, ann)					\
    rb_ary_push(current, ID2SYM(node->name))

#define F_GENTRY(name, ann) \
    rb_ary_push(current, ID2SYM((node->name)->id))

#define F_NODE(name, ann)			\
    add_to_parse_tree(self, current, node->name, NULL)

#define F_NODE_LOCALS(name, ann, locals)			\
    add_to_parse_tree(self, current, node->name, locals)

static VALUE
add_to_parse_tree(VALUE self, VALUE ary, NODE *node, ID *locals)
{
    VALUE current = Qnil;
    VALUE node_name;
    if (!node) {
	return D_NULL_NODE;
    }

    /* D_NODE_HEADER(node); */
    current = rb_ary_new();
    node_name = pt_node_name(NODE_NAME(node));
    rb_ary_push(ary, current);
    rb_ary_push(current, node_name);

    switch (nd_type(node)) {

      case NODE_BLOCK:
	ANN("statement sequence");
	ANN("format: [nd_head]; [nd_next]");
	ANN("example: foo; bar");
	F_NODE(nd_head, "current statement");
	LAST_NODE;
	F_NODE(nd_next, "next block");
	break;

      case NODE_IF:
	ANN("if statement");
	ANN("format: if [nd_cond] then [nd_body] else [nd_else] end");
	ANN("example: if x == 1 then foo else bar end");
	F_NODE(nd_cond, "condition expr");
	F_NODE(nd_body, "then clause");
	LAST_NODE;
	F_NODE(nd_else, "else clause");
	break;

      case NODE_CASE:
	ANN("case statement");
	ANN("format: case [nd_head]; [nd_body]; end");
	ANN("example: case x; when 1; foo; when 2; bar; else baz; end");
	F_NODE(nd_head, "case expr");
	LAST_NODE;
	F_NODE(nd_body, "when clauses");
	break;

      case NODE_WHEN:
	ANN("if statement");
	ANN("format: when [nd_head]; [nd_body]; (when or else) [nd_next]");
	ANN("example: case x; when 1; foo; when 2; bar; else baz; end");
	F_NODE(nd_head, "when value");
	F_NODE(nd_body, "when clause");
	LAST_NODE;
	F_NODE(nd_next, "next when clause");
	break;

      case NODE_OPT_N:
	ANN("wrapper for -n option");
	ANN("format: ruby -ne '[nd_body]' (nd_cond is `gets')");
	ANN("example: ruby -ne 'p $_'");
	goto loop;
      case NODE_WHILE:
	ANN("while statement");
	ANN("format: while [nd_cond]; [nd_body]; end");
	ANN("example: while x == 1; foo; end");
	goto loop;
      case NODE_UNTIL:
	ANN("until statement");
	ANN("format: until [nd_cond]; [nd_body]; end");
	ANN("example: until x == 1; foo; end");
      loop:
	rb_ary_push(current, (node->nd_state == 0) ? Qfalse : Qtrue);
	F_NODE_LOCALS(nd_cond, "condition", locals);
	LAST_NODE;
	if (node->nd_body) {
	    add_to_parse_tree(self, current,  node->nd_body, locals);
	} else {
	    rb_ary_push(current, Qnil);
	}
	break;

      case NODE_ITER:
	ANN("method call with block");
	ANN("format: [nd_iter] { [nd_body] }");
	ANN("example: 3.times { foo }");
	goto iter;
      case NODE_FOR:
	ANN("for statement");
	ANN("format: for * in [nd_iter] do [nd_body] end");
	ANN("example: for i in 1..3 do foo end");
	iter:
	F_NODE(nd_iter, "iteration receiver");
	LAST_NODE;
	F_NODE(nd_body, "body");
	break;
      case NODE_BREAK:
	ANN("for statement");
	ANN("format: break [nd_stts]");
	ANN("example: break 1");
	goto jump;
      case NODE_NEXT:
	ANN("next statement");
	ANN("format: next [nd_stts]");
	ANN("example: next 1");
	goto jump;
      case NODE_RETURN:
	ANN("return statement");
	ANN("format: return [nd_stts]");
	ANN("example: return 1");
        jump:
	LAST_NODE;
	F_NODE(nd_stts, "value");
	break;

      case NODE_REDO:
	ANN("redo statement");
	ANN("format: redo");
	ANN("example: redo");
	break;

      case NODE_RETRY:
	ANN("retry statement");
	ANN("format: retry");
	ANN("example: retry");
	break;

      case NODE_BEGIN:
	ANN("begin statement");
	ANN("format: begin; [nd_body]; end");
	ANN("example: begin; 1; end");
	LAST_NODE;
	F_NODE(nd_body, "body");
	break;

      case NODE_RESCUE:
	ANN("rescue clause");
	ANN("format: begin; [nd_body]; (rescue) [nd_resq]; else [nd_else]; end");
	ANN("example: begin; foo; rescue; bar; else; baz; end");
	F_NODE(nd_head, "body");
	F_NODE(nd_resq, "rescue clause list");
	LAST_NODE;
	F_NODE(nd_else, "rescue else clause");
	break;

      case NODE_RESBODY:
	ANN("rescue clause (cont'd)");
	ANN("format: rescue [nd_args]; [nd_body]; (rescue) [nd_head]");
	ANN("example: begin; foo; rescue; bar; else; baz; end");
	F_NODE(nd_args, "rescue exceptions");
	F_NODE(nd_body, "rescue clause");
	LAST_NODE;
	F_NODE(nd_head, "next rescue clause");
	break;

      case NODE_ENSURE:
	ANN("ensure clause");
	ANN("format: begin; [nd_head]; ensure; [nd_ensr]; end");
	ANN("example: begin; foo; ensure; bar; end");
	F_NODE(nd_head, "body");
	LAST_NODE;
	F_NODE(nd_ensr, "ensure clause");
	break;

      case NODE_AND:
	ANN("&& operator");
	ANN("format: [nd_1st] && [nd_2nd]");
	ANN("example: foo && bar");
	goto andor;
      case NODE_OR:
	ANN("|| operator");
	ANN("format: [nd_1st] || [nd_2nd]");
	ANN("example: foo && bar");
	andor:
	F_NODE(nd_1st, "left expr");
	LAST_NODE;
	F_NODE(nd_2nd, "right expr");
	break;

      case NODE_LASGN:
	ANN("local variable assignment");
	ANN("format: [nd_vid](lvar) = [nd_value]");
	ANN("example: x = foo");
	goto asgn;
      case NODE_DASGN:
	ANN("dynamic variable assignment (out of current scope)");
	ANN("format: [nd_vid](dvar) = [nd_value]");
	ANN("example: x = nil; 1.times { x = foo }");
	goto asgn;
      case NODE_DASGN_CURR:
	ANN("dynamic variable assignment (in current scope)");
	ANN("format: [nd_vid](current dvar) = [nd_value]");
	ANN("example: 1.times { x = foo }");
	goto asgn;
      case NODE_IASGN:
	ANN("instance variable assignment");
	ANN("format: [nd_vid](ivar) = [nd_value]");
	ANN("example: @x = foo");
	goto asgn;
      case NODE_CVASGN:
	ANN("class variable assignment");
	ANN("format: [nd_vid](cvar) = [nd_value]");
	ANN("example: @@x = foo");
	asgn:
	F_ID(nd_vid, "variable");
	LAST_NODE;
	F_NODE(nd_value, "rvalue");
	break;

      case NODE_GASGN:
	ANN("global variable assignment");
	ANN("format: [nd_entry](gvar) = [nd_value]");
	ANN("example: $x = foo");
	F_GENTRY(nd_entry, "global variable");
	LAST_NODE;
	F_NODE(nd_value, "rvalue");
	break;

      case NODE_OP_ASGN_AND:
	ANN("assignment with && operator");
	ANN("format: [nd_head] &&= [nd_value]");
	ANN("example: foo &&= bar");
	goto asgn_andor;
      case NODE_OP_ASGN_OR:
	ANN("assignment with || operator");
	ANN("format: [nd_head] ||= [nd_value]");
	ANN("example: foo ||= bar");
	asgn_andor:
	F_NODE(nd_head, "variable");
	LAST_NODE;
	F_NODE(nd_value, "rvalue");
	break;

      case NODE_OP_ASGN1:
	ANN("array assignment with operator");
	ANN("format: [nd_value] [ [nd_args->nd_body] ] [nd_vid]= [nd_args->nd_head]");
	ANN("example: ary[1] += foo");
	F_NODE(nd_recv, "receiver");
	F_ID(nd_vid, "operator");
	F_NODE(nd_args->nd_body, "index");
	LAST_NODE;
	F_NODE(nd_args->nd_head, "rvalue");
	break;

      case NODE_CALL:
	ANN("method invocation");
	ANN("format: [nd_recv].[nd_mid]([nd_args])");
	ANN("example: obj.foo(1)");
	F_ID(nd_mid, "method id");
	F_NODE(nd_recv, "receiver");
	LAST_NODE;
	F_NODE(nd_args, "arguments");
	break;

      case NODE_FCALL:
	ANN("function call");
	ANN("format: [nd_mid]([nd_args])");
	ANN("example: foo(1)");
	F_ID(nd_mid, "method id");
	LAST_NODE;
	F_NODE(nd_args, "arguments");
	break;

      case NODE_VCALL:
	ANN("function call with no argument");
	ANN("format: [nd_mid]");
	ANN("example: foo");
	F_ID(nd_mid, "method id");
	break;

      case NODE_SUPER:
	ANN("super invocation");
	ANN("format: super [nd_args]");
	ANN("example: super 1");
	LAST_NODE;
	F_NODE(nd_args, "arguments");
	break;

      case NODE_ZSUPER:
	ANN("super invocation with no argument");
	ANN("format: super");
	ANN("example: super");
	break;

      case NODE_ZARRAY:
	ANN("empty array constructor");
	ANN("format: []");
	ANN("example: []");
	break;

      case NODE_HASH:
	ANN("hash constructor");
	ANN("format: { [nd_head] }");
	ANN("example: { 1 => 2, 3 => 4 }");
	LAST_NODE;
	F_NODE(nd_head, "contents");
	break;

      case NODE_YIELD:
	ANN("yield invocation");
	ANN("format: yield [nd_head]");
	ANN("example: yield 1");
	LAST_NODE;
	F_NODE(nd_head, "arguments");
	break;

      case NODE_LVAR:
	ANN("local variable reference");
	ANN("format: [nd_vid](lvar)");
	ANN("example: x");
	goto var;
      case NODE_DVAR:
	ANN("dynamic variable reference");
	ANN("format: [nd_vid](dvar)");
	ANN("example: 1.times { x = 1; x }");
	goto var;
      case NODE_IVAR:
	ANN("instance variable reference");
	ANN("format: [nd_vid](ivar)");
	ANN("example: @x");
	goto var;
      case NODE_CONST:
	ANN("constant reference");
	ANN("format: [nd_vid](constant)");
	ANN("example: X");
	goto var;
      case NODE_CVAR:
	ANN("class variable reference");
	ANN("format: [nd_vid](cvar)");
	ANN("example: @@x");
        var:
	F_ID(nd_vid, "local variable");
	break;

      case NODE_GVAR:
	ANN("global variable reference");
	ANN("format: [nd_entry](gvar)");
	ANN("example: $x");
	F_GENTRY(nd_entry, "global variable");
	break;

      case NODE_LIT:
	ANN("literal");
	ANN("format: [nd_lit]");
	ANN("example: 1, /foo/");
	goto lit;
      case NODE_STR:
	ANN("string literal");
	ANN("format: [nd_lit]");
	ANN("example: 'foo'");
	goto lit;
      case NODE_XSTR:
	ANN("xstring literal");
	ANN("format: [nd_lit]");
	ANN("example: `foo`");
	lit:
	F_LIT(nd_lit, "literal");
	break;

      case NODE_DSTR:
	ANN("string literal with interpolation");
	ANN("format: [nd_lit]");
	ANN("example: \"foo#{ bar }baz\"");
	goto dlit;
      case NODE_DXSTR:
	ANN("xstring literal with interpolation");
	ANN("format: [nd_lit]");
	ANN("example: `foo#{ bar }baz`");
	goto dlit;
      case NODE_DREGX:
	ANN("regexp literal with interpolation");
	ANN("format: [nd_lit]");
	ANN("example: /foo#{ bar }baz/");
	goto dlit;
      case NODE_DREGX_ONCE:
	ANN("regexp literal with interpolation and once flag");
	ANN("format: [nd_lit]");
	ANN("example: /foo#{ bar }baz/o");
	goto dlit;
      case NODE_DSYM:
	ANN("symbol literal with interpolation");
	ANN("format: [nd_lit]");
	ANN("example: :\"foo#{ bar }baz\"");
	dlit:
	F_LIT(nd_lit, "literal");
	F_NODE(nd_next->nd_head, "preceding string");
	LAST_NODE;
	F_NODE(nd_next->nd_next, "interpolation");
	break;

      case NODE_EVSTR:
	ANN("interpolation expression");
	ANN("format: \"..#{ [nd_lit] }..\"");
	ANN("example: \"foo#{ bar }baz\"");
	LAST_NODE;
	F_NODE(nd_body, "body");
	break;

      case NODE_ALIAS:
	ANN("method alias statement");
	ANN("format: alias [u1.node] [u2.node]");
	ANN("example: alias bar foo");
	F_NODE(u1.node, "new name");
	LAST_NODE;
	F_NODE(u2.node, "old name");
	break;

      case NODE_VALIAS:
	ANN("global variable alias statement");
	ANN("format: alias [u1.id](gvar) [u2.id](gvar)");
	ANN("example: alias $y $x");
	F_ID(u1.id, "new name");
	F_ID(u2.id, "old name");
	break;

      case NODE_UNDEF:
	ANN("method alias statement");
	ANN("format: undef [u2.node]");
	ANN("example: undef foo");
	LAST_NODE;
	F_NODE(u2.node, "old name");
	break;

      case NODE_DOT2:
	ANN("range constructor (incl.)");
	ANN("format: [nd_beg]..[nd_end]");
	ANN("example: 1..5");
	goto dot;
      case NODE_DOT3:
	ANN("range constructor (excl.)");
	ANN("format: [nd_beg]...[nd_end]");
	ANN("example: 1...5");
	goto dot;
      case NODE_FLIP2:
	ANN("flip-flop condition (incl.)");
	ANN("format: [nd_beg]..[nd_end]");
	ANN("example: if (x==1)..(x==5); foo; end");
	goto dot;
      case NODE_FLIP3:
	ANN("flip-flop condition (excl.)");
	ANN("format: [nd_beg]...[nd_end]");
	ANN("example: if (x==1)...(x==5); foo; end");
	dot:
	F_NODE(nd_beg, "begin");
	LAST_NODE;
	F_NODE(nd_end, "end");
	break;

      case NODE_SELF:
	ANN("self");
	ANN("format: self");
	ANN("example: self");
	break;

      case NODE_NIL:
	ANN("nil");
	ANN("format: nil");
	ANN("example: nil");
	break;

      case NODE_TRUE:
	ANN("true");
	ANN("format: true");
	ANN("example: true");
	break;

      case NODE_FALSE:
	ANN("false");
	ANN("format: false");
	ANN("example: false");
	break;

      case NODE_COLON2:
	ANN("scoped constant reference");
	ANN("format: [nd_head]::[nd_mid]");
	ANN("example: M::C");
	F_ID(nd_mid, "constant name");
	LAST_NODE;
	F_NODE(nd_head, "receiver");
	break;

      case NODE_COLON3:
	ANN("top-level constant reference");
	ANN("format: ::[nd_mid]");
	ANN("example: ::Object");
	F_ID(nd_mid, "constant name");
	break;

      case NODE_DEFINED:
	ANN("defined? expression");
	ANN("format: defined?([nd_head])");
	ANN("example: defined?(foo)");
	F_NODE(nd_head, "expr");
	break;

      case NODE_SCOPE:
	ANN("new scope");
	ANN("format: [nd_tbl]: local table, [nd_args]: arguments, [nd_body]: body");
	F_NODE_LOCALS(nd_args, "arguments", node->nd_tbl);
	LAST_NODE;
	F_NODE_LOCALS(nd_body, "body", node->nd_tbl);
	break;
      default:
	rb_bug("dump_node: unknown node: %s", ruby_node_name(nd_type(node)));
    }
    return current;
}
