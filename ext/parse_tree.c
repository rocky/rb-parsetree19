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

   As such the macros below may seem weird because they are from node.c
   but try to do corresponding ParseTree actions.
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

#define F_LIT(name, ann) \
    rb_ary_push(current, node->name)

#define F_NODE(name, local) \
    add_to_parse_tree(self, current, node->name, local)


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

    D_NODE_HEADER(node);

    switch (nd_type(node)) {

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

      case NODE_SCOPE:
	ANN("new scope");
	ANN("format: [nd_tbl]: local table, [nd_args]: arguments, [nd_body]: body");
	F_NODE(nd_args, node->nd_tbl);
	LAST_NODE;
	F_NODE(nd_body, node->nd_tbl);
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

