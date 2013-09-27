/* What release we got? */
#define PARSETREE19_VERSION "0.3"
#include "../include/vm_core_mini.h"   /* Pulls in ruby.h and node.h */
#include "../include/ruby19_externs.h"

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

#include "add_to_parse_tree.inc"

static VALUE
parse_tree_for_node(VALUE self, NODE *node, VALUE /*bool*/ as_tree)
{
    VALUE result = rb_ary_new();

    if (Qfalse == as_tree)
	result = rb_parser_dump_tree(node, 0);
    else
	result = add_to_parse_tree(self, result, node, NULL);
    return (result);
}

static VALUE
parse_tree_for_iseq_internal(VALUE self, rb_iseq_t *iseq,
			     /*bool*/ VALUE as_tree)
{
    VALUE result = rb_ary_new();

    if (iseq && iseq->tree_node) {
	if (Qfalse == as_tree)
	    result = rb_parser_dump_tree(iseq->tree_node, 0);
	else
	    result = add_to_parse_tree(self, result, iseq->tree_node, NULL);
	return (result);
    } else
	return Qnil;
}

static VALUE
parse_tree_for_iseq(VALUE self, VALUE iseqval, /*Bool*/ VALUE as_tree)
{
    rb_iseq_t *iseq;

    GetISeqPtr(iseqval, iseq);
    return parse_tree_for_iseq_internal(self, iseq, as_tree);
}

/* Defined in Ruby 1.9 proc.c */
extern rb_iseq_t *rb_method_get_iseq(VALUE method);

static VALUE
parse_tree_for_method(VALUE self, VALUE method, VALUE as_tree)
{
    rb_iseq_t *iseq = rb_method_get_iseq(method);
    return parse_tree_for_iseq_internal(self, iseq, as_tree);
}

static VALUE
parse_tree_common(VALUE self, VALUE source, VALUE filename, VALUE line,
                  VALUE tree)
{
    VALUE tmp;
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

    return parse_tree_for_node(self, node, tree);
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
    rb_define_method(c, "parse_tree_for_iseq",
		     (VALUE(*)(ANYARGS))parse_tree_for_iseq, 2);
    rb_define_method(c, "parse_tree_for_method",
		     (VALUE(*)(ANYARGS))parse_tree_for_method, 2);
    rb_define_method(c, "parse_tree_for_str",
		     (VALUE(*)(ANYARGS))parse_tree_for_str, 3);
    rb_define_method(c, "parse_tree_dump",
		     (VALUE(*)(ANYARGS))parse_tree_dump, 3);
}
