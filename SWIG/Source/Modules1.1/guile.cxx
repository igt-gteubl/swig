/******************************************************************************
 * Simplified Wrapper and Interface Generator  (SWIG)
 *
 * Author : David Beazley
 *
 * Department of Computer Science
 * University of Chicago
 * 1100 E 58th Street
 * Chicago, IL  60637
 * beazley@cs.uchicago.edu
 *
 * Please read the file LICENSE for the copyright and terms by which SWIG
 * can be used and distributed.
 *****************************************************************************/

static char cvsroot[] = "$Header$";

/***********************************************************************
 * $Header$
 *
 * guile.cxx
 *
 * Definitions for adding functions to Guile
 ***********************************************************************/

#include "swig11.h"
#include "guile.h"

static char *guile_usage = (char*)"\
Guile Options (available with -guile)\n\
     -module name    - Set base name of module\n\
     -prefix name    - Use NAME as prefix [default \"gswig_\"]\n\
     -package name   - Set the path of the module [default NULL]\n\
     -Linkage lstyle - Use linkage protocol LSTYLE [default `ltdlmod']\n\
     -procdoc file   - Output procedure documentation to file\n\
\n\
  The module option does not create a guile module with a separate name\n\
  space.  It specifies the name of the initialization function and is\n\
  called a module here so that it is compadible with the rest of SWIG.\n\
\n\
  When unspecified, the default LSTYLE is `ltdlmod' for libtool ltdl\n\
  modules.  Other LSTYLE values are: `hobbit' for hobbit modules.\n\
\n";

// ---------------------------------------------------------------------
// GUILE ()
// ---------------------------------------------------------------------

GUILE::GUILE ()
{

  // Set class vars

  prefix = (char*)"gswig_";
  module = NULL;
  package = NULL;
  linkage = GUILE_LSTYLE_SIMPLE;
  procdoc = NULL;
  emit_setters = 0;
  struct_member = 0;
}

// ---------------------------------------------------------------------
// GUILE::parse_args(int argc, char *argv[])
//
// Parse arguments.
// ---------------------------------------------------------------------

void
GUILE::parse_args (int argc, char *argv[])
{
  int i, orig_len;

  sprintf (LibDir, "%s", "guile");

  // Look for certain command line options
  for (i = 1; i < argc; i++) {
    if (argv[i]) {
      if (strcmp (argv[i], "-help") == 0) {
	fputs (guile_usage, stderr);
	SWIG_exit (EXIT_SUCCESS);
      }
      else if (strcmp (argv[i], "-prefix") == 0) {
	if (argv[i + 1]) {
	  prefix = new char[strlen (argv[i + 1]) + 2];
	  strcpy (prefix, argv[i + 1]);
	  Swig_mark_arg (i);
	  Swig_mark_arg (i + 1);
	  i++;
	} else {
	  Swig_arg_error();
	}
      }
      else if (strcmp (argv[i], "-package") == 0) {
	if (argv[i + 1]) {
	  package = new char[strlen (argv[i + 1]) + 2];
	  strcpy (package, argv [i + 1]);
	  Swig_mark_arg (i);
	  Swig_mark_arg (i + 1);
	  i++;
	} else {
	  Swig_arg_error();
	}
      }
      /* Bogus upcase requirement due to top-level parsing not respecting
         language specification.  Top-level should stop when it sees "-guile"
         or other languages.  */
      else if (strcmp (argv[i], "-Linkage") == 0) {
        if (argv[i + 1]) {
          if (0 == strcmp (argv[i + 1], "ltdlmod"))
            linkage = GUILE_LSTYLE_LTDLMOD;
          else if (0 == strcmp (argv[i + 1], "hobbit"))
            linkage = GUILE_LSTYLE_HOBBIT;
          else
            Swig_arg_error ();
          Swig_mark_arg (i);
          Swig_mark_arg (i + 1);
          i++;
        } else {
          Swig_arg_error();
        }
      }
      else if (strcmp (argv[i], "-procdoc") == 0) {
	if (argv[i + 1]) {
	  procdoc = NewFile(argv[i + 1], "w");
	  Swig_mark_arg (i);
          Swig_mark_arg (i + 1);
	  i++;
	} else {
	  Swig_arg_error();
        }
      }
      else if (strcmp (argv[i], "-emit-setters") == 0) {
	emit_setters = 1;
	Swig_mark_arg (i);
      }
    }
  }

  // Make sure `prefix' ends in an underscore

  orig_len = strlen (prefix);
  if (prefix[orig_len - 1] != '_') {
    prefix[1 + orig_len] = 0;
    prefix[orig_len] = '_';
  }

  /* Add a symbol for this module */
  Preprocessor_define ((void *) "SWIGGUILE",0);

  /* Read in default typemaps */
  SWIG_config_file("guile.i");
}

// --------------------------------------------------------------------
// GUILE::initialize()
//
// Output initialization code that registers functions with the
// interface.
// ---------------------------------------------------------------------

void
GUILE::initialize (String *modname)
{
  printf ("Generating wrappers for Guile\n");

  Printf(f_runtime, "/* -*- buffer-read-only: t -*- vi: set ro: */\n");
  Swig_banner (f_runtime);

  Printf (f_runtime, "/* Implementation : GUILE */\n\n");

  // Write out directives and declarations

  if (NoInclude) {
    Printf(f_runtime, "#define SWIG_NOINCLUDE\n");
  }

  if (!module) {
    module = new char[Len(modname)+1];
    strcpy(module, Char(modname));
  }

  switch (linkage) {
  case GUILE_LSTYLE_SIMPLE:
    /* Simple linkage; we have to export the SWIG_init function. The user can
       rename the function by a #define. */
    Printf (f_runtime, "extern void\nSWIG_init (void)\n;\n");
    Printf (f_init, "extern void\nSWIG_init (void)\n{\n");
    break;
  default:
    /* Other linkage; we make the SWIG_init function static */
    Printf (f_runtime, "static void\nSWIG_init (void)\n;\n");
    Printf (f_init, "static void\nSWIG_init (void)\n{\n");
    break;
  }
  Printf (f_init, "\tSWIG_Guile_Init();\n");
}

void
GUILE::emit_linkage (char *module_name)
{
  DOHString *module_func = NewString("");

  Printv(module_func,module_name,0);
  Replace(module_func,"-", "_", DOH_REPLACE_ANY);

  switch (linkage) {
  case GUILE_LSTYLE_SIMPLE:
    Printf (f_init, "\n/* Linkage: simple */\n");
    break;
  case GUILE_LSTYLE_LTDLMOD:
    Printf (f_init, "\n/* Linkage: ltdlmod */\n");
    Replace(module_func,"/", "_", DOH_REPLACE_ANY);
    Insert(module_func,0, "scm_init_");
    Append(module_func,"_module");
    Printf (f_init, "SCM\n%s (void)\n{\n", module_func);
    {
      DOHString *mod = NewString(module_name);
      Replace(mod,"/", " ", DOH_REPLACE_ANY);
      Printf (f_init, "    scm_register_module_xxx (\"%s\", (void *) SWIG_init);\n",
               mod);
      Printf (f_init, "    return SCM_UNSPECIFIED;\n");
      Delete(mod);
    }
    Printf (f_init, "}\n");
    break;
  case GUILE_LSTYLE_HOBBIT:
    Printf (f_init, "\n/* Linkage: hobbit */\n");
    Replace(module_func,"/", "_slash_", DOH_REPLACE_ANY);
    Insert(module_func,0, "scm_init_");
    Printf (f_init, "SCM\n%s (void)\n{\n", module_func);
    {
      DOHString *mod = NewString(module_name);
      Replace(mod,"/", " ", DOH_REPLACE_ANY);
      Printf (f_init, "    scm_register_module_xxx (\"%s\", (void *) SWIG_init);\n",
               mod);
      Printf (f_init, "    return SCM_UNSPECIFIED;\n");
      Delete(mod);
    }
    Printf (f_init, "}\n");
    break;
  default:
    abort();                            // for now
  }
  Delete(module_func);
}

// ---------------------------------------------------------------------
// GUILE::close(void)
//
// Wrap things up.  Close initialization function.
// ---------------------------------------------------------------------

void
GUILE::close (void)
{
  SwigType_emit_type_table (f_runtime, f_wrappers);

  Printf (f_init, "SWIG_Guile_RegisterTypes(swig_types);\n");
  Printf (f_init, "}\n\n");
  char module_name[256];

  if (!module)
    sprintf(module_name, "swig");
  else {
    if (package)
      sprintf(module_name,"%s/%s", package,module);
    else
      strcpy(module_name,module);
  }
  emit_linkage (module_name);

  if (procdoc) {
    Delete(procdoc);
    procdoc = NULL;
  }
}

// ----------------------------------------------------------------------
// get_pointer()
//
// Emits code to get a pointer from a parameter and do type checking.
// parm is the parameter number.   This function is only used
// in create_function().
// ----------------------------------------------------------------------

static void
get_pointer (char *iname, int parm, SwigType *t,
	     Wrapper *f, DOHString_or_char *proc_name,
	     int num_scheme_parm)
{
  SwigType_remember(t);
  /* Pointers are smobs */
  Printf(f, "    if (SWIG_Guile_GetPtr(s_%d,(void **) &arg%d", parm, parm);
  if (SwigType_type(t) == T_VOID)
    Printf(f, ", NULL)) {\n");
  else
    Printv(f, ", SWIGTYPE", SwigType_manglestr(t), ")) {\n", 0);
  /* Raise exception */
  Printv(f,
	 tab8,
         "scm_wrong_type_arg(\"",proc_name, "\", ",
	 0);
  Printf(f,"%d, s_%d);\n", num_scheme_parm, parm);
  Printv(f, tab4, "}\n", 0);
}

/* Return true iff T is a pointer type */

static int
is_a_pointer (SwigType *t)
{
  return SwigType_ispointer(SwigType_typedef_resolve_all(t));
}

/* Same as Swig_typemap_lookup but fall back to `int' when `enum' is
   requested -- enum handling is somewhat broken in the 1.1 parser.
   But we don't want to change it now since it is deprecated. */

static char *
guile_typemap_lookup(const char *op, SwigType *type, const String_or_char *pname, String_or_char *source,
		     String_or_char *target, Wrapper *f)
{
  char *tm;
  tm = Swig_typemap_lookup((char*) op, type, (char*)pname, source, target, f);
  if (!tm) {
    SwigType *base = SwigType_typedef_resolve_all(type);
    if (strncmp(Char(base), "enum ", 5)==0)
      tm = Swig_typemap_lookup((char*) op, (char*) "int", (char*)pname, source, target, f);
  }
  return tm;
}

/* Lookup a typemap, replace all relevant parameters and write it to
   the given generalized file. Return 0 if no typemap found. */

static int
guile_do_typemap(DOHFile *file, const char *op,
		 SwigType *type, const String_or_char *arg,
		 String_or_char *source, String_or_char *target,
		 int argnum, DOHString *name, Wrapper *f,
		 int nonewline_p)
{
  char *tm;
  if ((tm = guile_typemap_lookup(op, type, arg,
				 source, target, f))) {
    String *s = NewString(tm);
    char argnum_s[10];
    sprintf(argnum_s, "%d", argnum);
    Replace(s,"$argnum", argnum_s, DOH_REPLACE_ANY);
    Replace(s,"$arg",    arg,      DOH_REPLACE_ANY);
    Replace(s,"$name",   name,     DOH_REPLACE_ANY);
    if (nonewline_p)
      Printv(file, s, 0);
    else Printv(file, s, "\n", 0);
    Delete(s);
    return 1;
  }
  else return 0;
}

/* Lookup a documentation typemap, replace all relevant parameters and
   write it to the given generalized file, providing a sensible
   default value. */

static void
guile_do_doc_typemap(DOHFile *file, const char *op,
		     SwigType *type, const String_or_char *arg,
		     int argnum, DOHString *name, Wrapper *f)
{
  if (!guile_do_typemap(file, op, type, arg,
			NULL, NULL, argnum, name, f, 1)) {
    /* FIXME: Can't we provide this default via a typemap as well? */
    String *s = NewString(SwigType_str(type, 0));
    Chop(s);
    if (arg) Printf(file, "(%s <%s>)", arg, s);
    else Printf(file, "<%s>", s);
    Delete(s);
  }
}

/* Report an error handling the given type. */

static void
throw_unhandled_guile_type_error (SwigType *d)
{
  Printf (stderr, "%s:%d. Unable to handle type %s.\n",Getfile(d), Getline(d), SwigType_str(d,0));
}

// ----------------------------------------------------------------------
// GUILE::create_function()
// Create a function declaration and register it with the interpreter.
// ----------------------------------------------------------------------

void
GUILE::function (DOH *node) {
  char *name, *iname;
  SwigType *d;
  ParmList *l;
  Parm *p;
  DOHString *proc_name = 0;
  char source[256], target[256];
  Wrapper *f = NewWrapper();;
  String *cleanup = NewString("");
  String *outarg = NewString("");
  String *signature = NewString("");
  String *returns = NewString("");
  int returns_list = 0;
  String *tmp = NewString("");
  int i;
  int numargs = 0;
  int numopt = 0;

  name = GetChar(node,"name");
  iname = GetChar(node,"scriptname");
  d = Getattr(node,"type");
  l = Getattr(node,"parms");

  // Make a wrapper name for this
  char * wname = new char [strlen (prefix) + strlen (iname) + 2];
  sprintf (wname, "%s%s", prefix, iname);

  // Build the name for scheme.
  proc_name = NewString(iname);
  Replace(proc_name,"_", "-", DOH_REPLACE_ANY);

  /* Emit locals etc. into f; figure out which args to ignore */
  emit_args (node, f);

  /* Declare return variable */

  Wrapper_add_local (f,"gswig_result", "SCM gswig_result");

  if (procdoc)
    guile_do_doc_typemap(returns, "outdoc", d, NULL,
			 0, proc_name, f);

  /* Open prototype and signature */
  
  /* <DB> The function prototype must be produced first */

  Printv(f, "static SCM\n", wname," (", 0);
  for (p = l, i = 0; p; p = Getnext(p), i++) {
    if (i != 0) Printf(f,", ");
    Printf(f,"SCM s_%d", i);
  }
  Printf(f,")\n{\n");

  /* Define the scheme name in C. This define is used by several Guile
     macros. */

  Printv(f,"#define FUNC_NAME \"", proc_name, "\"\n", 0);

  Printv(signature, "(", proc_name, 0);
  /* Now write code to extract the parameters */

  for (p = l, i = 0; p; p=Getnext(p), i++) {
    SwigType *pt = Gettype(p);
    String   *pn = Getname(p);
    int opt_p = (Getvalue(p)
		 || Swig_typemap_search((char*)"default",pt,pn));

    // Produce names of source and target
    sprintf(source,"s_%d",i);
    sprintf(target,"%s", Char(Getlname(p)));

    // Handle parameter types.

    if (Getignore(p))
      Printv(f, "/* ", pn, " ignored... */\n", 0);
    else {
      if (opt_p) {
	numopt++;
	Printf(f,"    if (s_%d != GH_NOT_PASSED) {\n", i);
      }
      ++numargs;
      if (guile_do_typemap(f, "in", pt, pn,
			   source, target, numargs, proc_name, f, 0)) {
	/* nothing to do */
      }
      else if (is_a_pointer(pt)) {
        get_pointer (iname, i, pt, f, proc_name, numargs);
      }
      else {
        throw_unhandled_guile_type_error (pt);
      }
      if (procdoc) {
	/* Add to signature */
	Printf(signature, " ");
	guile_do_doc_typemap(signature, "indoc", pt, pn,
			     numargs, proc_name, f);
      }
      if (opt_p)
	Printf(f,"    }\n");
    }

    /* Check if there are any constraints. */

    guile_do_typemap(f, "check", pt, pn,
		     source, target, numargs, proc_name, f, 0);

    /* Pass output arguments back to the caller. */

    guile_do_typemap(outarg, "argout", pt, pn,
		     source, target, numargs, proc_name, f, 0);

    if (procdoc) {
      /* Document output arguments */
      Clear(tmp);
      if (guile_do_typemap(tmp, "argoutdoc", pt, pn,
			   source, target, numargs, proc_name, f, 1)) {
	if (Len(returns) == 0) { /* unspecified -> singleton */
	  Printv(returns, tmp, 0);
	}
	else { /* append to list */
	  Printv(returns, " ", tmp, 0);
	  returns_list = 1;
	}
      }
    }

    // free up any memory allocated for the arguments.

    guile_do_typemap(cleanup, "freearg", pt, pn,
		     source, target, numargs, proc_name, f, 0);
  }

  /* Close prototype and signature */

  Printv(signature, ")\n", 0);


  // Now write code to make the function call
  Printv(f, tab4, "gh_defer_ints();\n", 0);
  emit_func_call (node, f);
  Printv(f, tab4, "gh_allow_ints();\n", 0);

  // Now have return value, figure out what to do with it.

  if (guile_do_typemap(f, "out", d, name,
		       (char*)"result", (char*)"gswig_result",
		       0, proc_name, f, 0)) {
    /* nothing */
  }
  else if (is_a_pointer(d)) {
    SwigType_remember(d);
    Printv(f, tab4,
           "gswig_result = SWIG_Guile_MakePtr (",
           "result, ",
           "SWIGTYPE", SwigType_manglestr(d),
           ");\n",
	   0);
  }
  else {
    throw_unhandled_guile_type_error (d);
  }

  // Dump the argument output code
  Printv(f,outarg,0);

  // Dump the argument cleanup code
  Printv(f,cleanup,0);

  // Look for any remaining cleanup

  if (NewObject) {
    guile_do_typemap(f, "newfree", d, iname,
		     (char*)"result", (char*)"", 0, proc_name, f, 0);
  }

  // Free any memory allocated by the function being wrapped..

  guile_do_typemap(f, "ret", d, name,
		   (char*)"result", (char*)"", 0, proc_name, f, 0);

  // Wrap things up (in a manner of speaking)

  Printv(f, "return gswig_result;\n", 0);

  // Undefine the scheme name

  Printf(f, "#undef FUNC_NAME\n");
  Printf(f, "}\n");

  Printf(f_wrappers,"%s", f);

  if (numargs > 10) {
    int i;
    /* gh_new_procedure would complain: too many args */
    /* Build a wrapper wrapper */
    Printv(f_wrappers, "static SCM\n", wname,"_rest (SCM rest)\n", 0);
    Printv(f_wrappers, "{\n", 0);
    Printf(f_wrappers, "SCM arg[%d];\n", numargs);
    Printf(f_wrappers, "SWIG_Guile_GetArgs (arg, rest, %d, %d, \"%s\");\n",
	   numargs-numopt, numopt, proc_name);
    Printv(f_wrappers, "return ", wname, "(", 0);
    Printv(f_wrappers, "arg[0]", 0);
    for (i = 1; i<numargs; i++)
      Printf(f_wrappers, ", arg[%d]", i);
    Printv(f_wrappers, ");\n", 0);
    Printv(f_wrappers, "}\n", 0);
    /* Register it */
    Printf (f_init, "gh_new_procedure(\"%s\", %s_rest, 0, 0, 1);\n",
             proc_name, wname, numargs-numopt, numopt);
  }
  else if (emit_setters && struct_member && strlen(Char(proc_name))>3) {
    int len = Len(proc_name);
    const char *pc = Char(proc_name);
    /* MEMBER-set and MEMBER-get functions. */
    int is_setter = (pc[len - 3] == 's');
    if (is_setter) {
      Printf(f_init, "SCM setter = ");
      struct_member = 2; /* have a setter */
    }
    else Printf(f_init, "SCM getter = ");
    Printf (f_init, "gh_new_procedure(\"%s\", %s, %d, %d, 0);\n",
	    proc_name, wname, numargs-numopt, numopt);
    if (!is_setter) {
      /* Strip off "-get" */
      char *pws_name = (char*) malloc(sizeof(char) * (len - 3));
      strncpy(pws_name, pc, len - 3);
      pws_name[len - 4] = 0;
      if (struct_member==2) { 
	/* There was a setter, so create a procedure with setter */
	Printf (f_init, "gh_define(\"%s\", "
		"scm_make_procedure_with_setter(getter, setter));\n",
		pws_name);
      }
      else {
	/* There was no setter, so make an alias to the getter */
	Printf (f_init, "gh_define(\"%s\", getter);\n",
		pws_name);
      }
      free(pws_name);
    }
  }
  else {
    /* Register the function */
    Printf (f_init, "gh_new_procedure(\"%s\", %s, %d, %d, 0);\n",
	    proc_name, wname, numargs-numopt, numopt);
  }
  if (procdoc) {
    /* Write out procedure documentation */
    Printv(signature, "Returns ", 0);
    if (Len(returns)==0) Printv(signature, "unspecified", 0);
    else if (returns_list) Printv(signature, "list (", returns, ")", 0);
    else Printv(signature, returns, 0);
    Printv(signature, "\n", 0);
    Printv(procdoc, "\f\n", signature, 0);
  }

  Delete(proc_name);
  Delete(outarg);
  Delete(cleanup);
  Delete(signature);
  Delete(returns);
  Delete(tmp);
  Delete(f);
  delete[] wname;
}

// -----------------------------------------------------------------------
// GUILE::variable()
//
// Create a link to a C variable.
// This creates a single function PREFIX_var_VARNAME().
// This function takes a single optional argument.   If supplied, it means
// we are setting this variable to some value.  If omitted, it means we are
// simply evaluating this variable.  Either way, we return the variables
// value.
// -----------------------------------------------------------------------

void
GUILE::variable (DOH *node)
{
  char *name, *iname;
  SwigType *t;

  DOHString *proc_name;
  char  var_name[256];
  char  *tm;
  Wrapper *f;


  name = GetChar(node,"name");
  iname = GetChar(node,"scriptname");
  t = Getattr(node,"type");

  f = NewWrapper();
  // evaluation function names

  sprintf (var_name, "%svar_%s", prefix, iname);

  // Build the name for scheme.
  proc_name = NewString(iname);
  Replace(proc_name,"_", "-",DOH_REPLACE_ANY);

  if ((SwigType_type(t) != T_USER) || (is_a_pointer(t))) {

    Printf (f_wrappers, "SCM %s(SCM s_0) {\n", var_name);

    if (!(ReadOnly) && SwigType_type(t) == T_STRING) {
      Printf (f_wrappers, "\t char *_temp;\n");
      Printf (f_wrappers, "\t int  _len;\n");
    }
    Printf (f_wrappers, "\t SCM gswig_result;\n");

    // Check for a setting of the variable value

    Printf (f_wrappers, "\t if (s_0 != GH_NOT_PASSED) {\n");

    // Yup. Extract the type from s_0 and set variable value

    if (ReadOnly) {
      Printf (f_wrappers, "\t\t scm_misc_error(\"%s\", "
	       "\"Unable to set %s. Variable is read only.\", SCM_EOL);\n",
	       proc_name, proc_name);
    }
    else if ((tm = guile_typemap_lookup ("varin",
                                   t, name, (char*)"s_0", name, f))) {
      Printf (f_wrappers, "%s\n", tm);
    }
    else if (is_a_pointer(t)) {
      if (SwigType_type(t) == T_STRING) {
        Printf (f_wrappers, "\t\t _temp = gh_scm2newstr(s_0, &_len);\n");
        Printf (f_wrappers, "\t\t if (%s) { free(%s);}\n", name, name);
        Printf (f_wrappers, "\t\t %s = (char *) "
                 "malloc((_len+1)*sizeof(char));\n", name);
        Printf (f_wrappers, "\t\t strncpy(%s,_temp,_len);\n", name);
        Printf (f_wrappers, "\t\t %s[_len] = 0;\n", name);
      } else {
        // Set the value of a pointer
        Printf (f_wrappers, "\t if (SWIG_Guile_GetPtr(s_0, "
                 "(void **) &%s, ", name);
        if (SwigType_type(t) == T_VOID)
          Printf (f_wrappers, "NULL)) {\n");
        else
          Printf (f_wrappers, "SWIGTYPE%s)) {\n", SwigType_manglestr(t));
	/* Raise exception */
	Printf(f_wrappers, "\tscm_wrong_type_arg(\"%s\", "
		"%d, s_0);\n", proc_name, 1);
        Printf (f_wrappers, "\t}\n");
      }
    }
    else {
      throw_unhandled_guile_type_error (t);
    }
    Printf (f_wrappers, "\t}\n");

    // Now return the value of the variable (regardless
    // of evaluating or setting)

    if ((tm = guile_typemap_lookup ("varout",
                              t, name, name, (char*)"gswig_result", f))) {
      Printf (f_wrappers, "%s\n", tm);
    }
    else if (is_a_pointer(t)) {
      if (SwigType_type(t) == T_STRING) {
        Printf (f_wrappers, "\t gswig_result = gh_str02scm(%s);\n", name);
      } else {
        // Is an ordinary pointer type.
        Printf (f_wrappers, "\t gswig_result = SWIG_Guile_MakePtr ("
                 "%s, SWIGTYPE%s);\n", name, SwigType_manglestr(t));
      }
    }
    else {
      throw_unhandled_guile_type_error (t);
    }
    Printf (f_wrappers, "\t return gswig_result;\n");
    Printf (f_wrappers, "}\n");

    // Now add symbol to the Guile interpreter

    if (!emit_setters
	|| ReadOnly) {
      /* Read-only variables become a simple procedure returning the
	 value. */
      Printf (f_init, "\t gh_new_procedure(\"%s\", %s, 0, 1, 0);\n",
	      proc_name, var_name);
    }
    else {
      /* Read/write variables become a procedure with setter. */
      Printf (f_init, "\t{ SCM p = gh_new_procedure(\"%s\", %s, 0, 1, 0);\n",
	      proc_name, var_name);
      Printf (f_init, "\t  gh_define(\"%s\", "
	      "scm_make_procedure_with_setter(p, p)); }\n",
	      proc_name);
    }

    if (procdoc) {
      /* Compute documentation */
      String *signature = NewString("");

      if (ReadOnly) {
	Printv(signature, "(", proc_name, ")\n", 0);
	Printv(signature, "Returns constant ", 0);
	guile_do_doc_typemap(signature, "varoutdoc", t, NULL,
			     0, proc_name, f);
	Printv(signature, "\n", 0);
      }
      else {
	Printv(signature, "(", proc_name,
	       " #:optional ", 0);
	guile_do_doc_typemap(signature, "varindoc", t, "new-value",
			     1, proc_name, f);
	Printv(signature, ")\n", 0);
	Printv(signature, "If NEW-VALUE is provided, "
	       "set C variable to this value.\n", 0);
	Printv(signature, "Returns variable value ", 0);
	guile_do_doc_typemap(signature, "varoutdoc", t, NULL,
			     0, proc_name, f);
	Printv(signature, "\n", 0);
      }
      Printv(procdoc, "\f\n", signature, 0);
      Delete(signature);
    }

  } else {
    Printf (stderr, "%s:%d. ** Warning. Unable to link with "
             " type %s (ignored).\n",
             Getfile(node), Getline(node), SwigType_str(t,0));
  }
  Delete(proc_name);
  Delete(f);
}

// -----------------------------------------------------------------------
// GUILE::constant()
//
// Makes a constant.   Not sure how this is really supposed to work.
// I'm going to fake out SWIG and create a variable instead.
// ------------------------------------------------------------------------

void
GUILE::constant(DOH *node)
{
  char   *name;
  SwigType *type;
  char   *value;
  int OldStatus = ReadOnly;      // Save old status flags
  DOHString *proc_name;
  char   var_name[256];
  DOHString *rvalue;
  char   *tm;
  Wrapper *f;

  name = GetChar(node,"name");
  type = Getattr(node,"type");
  value = GetChar(node,"value");

  f = NewWrapper();
  ReadOnly = 1;      // Enable readonly mode.

  // Make a static variable;

  sprintf (var_name, "%sconst_%s", prefix, name);

  // Build the name for scheme.
  proc_name = NewString(name);
  Replace(proc_name,"_", "-", DOH_REPLACE_ANY);

  if ((SwigType_type(type) == T_USER) && (!is_a_pointer(type))) {
    Printf (stderr, "%s:%d.  Unsupported constant value.\n",
             Getfile(node), Getline(node));
    return;
  }

  // See if there's a typemap

  if (SwigType_type(type) == T_STRING) {
    rvalue = NewStringf("\"%s\"", value);
  } else if (SwigType_type(type) == T_CHAR) {
    rvalue = NewStringf("\'%s\'", value);
  } else {
    rvalue = NewString(value);
  }
  if ((tm = guile_typemap_lookup ("const", type, name,
                            Char(rvalue), name, f))) {
    Printf (f_init, "%s\n", tm);
  } else {
    // Create variable and assign it a value

    Printf (f_header, "static %s %s = ", SwigType_lstr(type,0), var_name);
    if (SwigType_type(type) == T_STRING) {
      Printf (f_header, "\"%s\";\n", value);
    } else if (SwigType_type(type) == T_CHAR) {
      Printf (f_header, "\'%s\';\n", value);
    } else {
      Printf (f_header, "%s;\n", value);
    }
    // Now create a variable declaration

    Hash *nnode = Copy(node);
    Setattr(nnode,"name",var_name);
    variable(nnode);
    Delete(nnode);
    ReadOnly = OldStatus;
  }
  Delete(proc_name);
  Delete(rvalue);
  Delete(f);
}

void GUILE::cpp_variable(DOH *node)
{
  if (emit_setters) {
    struct_member = 1;
    Printf(f_init, "{\n");
    Language::cpp_variable(node);
    Printf(f_init, "}\n");
    struct_member = 0;
  }
  else {
    /* Only emit traditional VAR-get and VAR-set procedures */
    Language::cpp_variable(node);
  }
}
