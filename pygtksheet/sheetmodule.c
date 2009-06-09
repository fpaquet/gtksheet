#include <pygobject.h>
void sheet_add_constants(PyObject *module, const gchar *strip_prefix);
void sheet_register_classes (PyObject *d); 
extern PyMethodDef sheet_functions[];
 
DL_EXPORT(void)
initsheet(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("sheet", sheet_functions);
    d = PyModule_GetDict (m);
 
    sheet_register_classes (d);
    sheet_add_constants(m, "GTK_");
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module sheet");
    }
}

