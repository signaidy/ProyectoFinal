#pragma once
#include <Python.h>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

// Simple C++ wrapper for matplotlib (requires Python with matplotlib installed)
namespace matplotlibcpp {
    namespace detail {
        struct _import_guard {
            _import_guard(){ Py_Initialize(); }
            ~_import_guard(){ if (Py_IsInitialized()) Py_Finalize(); }
        };
        static _import_guard guard;


        inline PyObject* import(const std::string& name) {
            PyObject* py_name = PyUnicode_FromString(name.c_str());
            PyObject* module = PyImport_Import(py_name);
            Py_DECREF(py_name);
            if(!module) throw std::runtime_error("Cannot import " + name);
            return module;
        }
    }

    // Simple plotter class
    class Plotter {
        PyObject* plt_ = nullptr;
        public:
        // Constructor: import matplotlib.pyplot
        Plotter(){
            PyObject* mpl = detail::import("matplotlib"); Py_DECREF(mpl);
            plt_ = detail::import("matplotlib.pyplot");
        }
        ~Plotter(){ if(plt_) Py_DECREF(plt_); }

        // Create a new figure with optional title
        void figure(const std::string& title=""){
            PyObject_CallMethod(plt_, "figure", NULL);
            if(!title.empty()){
            PyObject* args = PyTuple_Pack(1, PyUnicode_FromString(title.c_str()));
            PyObject_CallMethod(plt_, "title", "O", args);
            Py_DECREF(args);
            }
        }
        
        void bar(const std::vector<std::string>& labels, const std::vector<double>& vals){
            // Simple bar via Python: positions = range(n); plt.bar(positions, vals); plt.xticks(positions, labels, rotation=45, ha='right')
            PyObject* py_labels = PyList_New(labels.size());
            PyObject* py_vals = PyList_New(vals.size());
            for(size_t i=0;i<labels.size();++i){
                PyList_SetItem(py_labels, i, PyUnicode_FromString(labels[i].c_str()));
            }
            for(size_t i=0;i<vals.size();++i){
                PyList_SetItem(py_vals, i, PyFloat_FromDouble(vals[i]));
            }
            PyObject* kwargs = PyDict_New();
            PyDict_SetItemString(kwargs, "rotation", PyLong_FromLong(45));
            PyObject* res = PyObject_CallMethod(plt_, "bar", "OO", py_labels, py_vals);
            Py_XDECREF(res);
            res = PyObject_CallMethod(plt_, "xticks", "OO", py_labels, py_labels);
            Py_XDECREF(res);
            Py_DECREF(py_labels); Py_DECREF(py_vals); Py_DECREF(kwargs);
        }
        // Simple xy plot
        void plot(const std::vector<double>& x, const std::vector<double>& y){
            PyObject* py_x = PyList_New(x.size());
            PyObject* py_y = PyList_New(y.size());
            for(size_t i=0;i<x.size();++i){ PyList_SetItem(py_x, i, PyFloat_FromDouble(x[i])); }
            for(size_t i=0;i<y.size();++i){ PyList_SetItem(py_y, i, PyFloat_FromDouble(y[i])); }
            PyObject* res = PyObject_CallMethod(plt_, "plot", "OO", py_x, py_y);
            Py_XDECREF(res);
            Py_DECREF(py_x); Py_DECREF(py_y);
        }
        void ylabel(const std::string& s){ PyObject_CallMethod(plt_, "ylabel", "s", s.c_str()); }
        void xlabel(const std::string& s){ PyObject_CallMethod(plt_, "xlabel", "s", s.c_str()); }
        void tight_layout(){ PyObject_CallMethod(plt_, "tight_layout", NULL); }
        void savefig(const std::string& path){ PyObject_CallMethod(plt_, "savefig", "s", path.c_str()); }
        void clf(){ PyObject_CallMethod(plt_, "clf", NULL); }
    };
}