/* What release we got? */
#define PARSETREE19_VERSION "0.1"  
#include "../include/vm_core_mini.h"   /* Pulls in ruby.h and node.h */
#include "../include/ruby19_externs.h" 

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

#define F_NODE(name, ann, locals)			\
    add_to_parse_tree(self, current, node->name, locals)

/* Turn a Ruby node string, e.g. "NODE_FALSE" into a symbol that ParseTree
   uses, e.g. :false. */
static VALUE 
pt_node_name(const char *name)
{
    if (0 == strncmp(name, "NODE_", strlen("NODE_"))) { 
	const char * name_sans_node = &name[strlen("NODE_")];
	return 
	    rb_funcall(
		rb_funcall(rb_str_new2(name_sans_node), rb_intern("downcase"), 
			   0),
		rb_intern("to_sym"), 0);
    } else {
	rb_bug("pt_node_name: node %s name should start with NODE_", name);
	return Qnil;
    }
}

static VALUE 
wrap_into_node(const char * name, VALUE val) 
{
    VALUE n = rb_ary_new();
    rb_ary_push(n, pt_node_name(name));
    if (val) rb_ary_push(n, val);
    return n;
}

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
	F_NODE(nd_stts, "value", NULL);
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
	F_NODE(nd_value, "rvalue", NULL);
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
	F_NODE(nd_head, "variable", NULL);
	LAST_NODE;
	F_NODE(nd_value, "rvalue", NULL);
	break;

      case NODE_YIELD:
	ANN("yield invocation");
	ANN("format: yield [nd_head]");
	ANN("example: yield 1");
	LAST_NODE;
	F_NODE(nd_head, "arguments", NULL);
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
	F_NODE(nd_next->nd_head, "preceding string", NULL);
	LAST_NODE;
	F_NODE(nd_next->nd_next, "interpolation", NULL);
	break;

      case NODE_EVSTR:
	ANN("interpolation expression");
	ANN("format: \"..#{ [nd_lit] }..\"");
	ANN("example: \"foo#{ bar }baz\"");
	LAST_NODE;
	F_NODE(nd_body, "body", NULL);
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
	F_NODE(nd_head, "receiver", NULL);
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
	F_NODE(nd_head, "expr", NULL);
	break;

      case NODE_SCOPE:
	ANN("new scope");
	ANN("format: [nd_tbl]: local table, [nd_args]: arguments, [nd_body]: body");
	F_NODE(nd_args, "arguments", node->nd_tbl);
	LAST_NODE;
	F_NODE(nd_body, "body", node->nd_tbl);
	break;
      default:
	rb_bug("dump_node: unknown node: %s", ruby_node_name(nd_type(node)));
    }
    return current;
}

static VALUE 
parse_tree_common(VALUE self, VALUE source, VALUE filename, VALUE line,
                  VALUE tree)
{
    VALUE tmp;
    VALUE result = rb_ary_new();
    NODE *node = NULL;

#ifdef FIXED
    rb_thread_t *th = GET_THREAD();
    int critical;
#endif

    tmp = rb_check_string_type(filename);
    if (NIL_P(tmp)) {
	filename = rb_str_new2("(string)");
    }

    if (NIL_P(line)) {
	line = LONG2FIX(1);
    }

#ifdef FIXED
    ruby_nerrs = 0;
    critical = rb_thread_critical;
    rb_thread_critical = Qtrue;
    th->parse_in_eval++;
#endif

    StringValue(source);
    node = rb_compile_string(StringValuePtr(filename), source, NUM2INT(line));

#ifdef FIXED
    th->parse_in_eval--;
    rb_thread_critical = critical;
    if (ruby_nerrs > 0) {
	ruby_nerrs = 0;
	rb_exc_raise(th->errinfo);
    }
#endif

    if (Qfalse == tree)
	result = rb_parser_dump_tree(node, 0);
    else
	result = add_to_parse_tree(self, result, node, NULL);
    return (result);

}

static VALUE 
parse_tree_for_str(VALUE self, VALUE source, VALUE filename, VALUE line) 
{
    return parse_tree_common(self, source, filename, line, Qtrue);
}

static VALUE 
parse_tree_dump(VALUE self, VALUE source, VALUE filename, VALUE line) 
{
    return parse_tree_common(self, source, filename, line, Qfalse);
}

void Init_parse_tree(void) 
{
    VALUE c = rb_define_class("ParseTree19", rb_cObject);
    /* Probably needs more changes to Ruby 1.9:
    rb_define_method(c, "parse_tree_for_meth", 
		     (VALUE(*)(ANYARGS))parse_tree_for_meth, 3);
    */
    rb_define_method(c, "parse_tree_for_str", 
		     (VALUE(*)(ANYARGS))parse_tree_for_str, 3);
    rb_define_method(c, "parse_tree_dump", 
		     (VALUE(*)(ANYARGS))parse_tree_dump, 3);
}

