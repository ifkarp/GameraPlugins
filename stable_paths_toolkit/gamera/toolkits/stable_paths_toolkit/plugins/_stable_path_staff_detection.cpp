
        
    
  #include "gameramodule.hpp"
  #include "knnmodule.hpp"

        #include "stable_path_staff_detection.hpp"
  
    #include <string>
  #include <stdexcept>
  #include "Python.h"
  #include <list>

  using namespace Gamera;
  
        
      extern "C" {
#ifndef _MSC_VER
    void init_stable_path_staff_detection(void);
#endif
                  static PyObject* call_returnGraphWeights(PyObject* self, PyObject* args);
                        static PyObject* call_copyImage(PyObject* self, PyObject* args);
            }

          static PyMethodDef _stable_path_staff_detection_methods[] = {
                  { CHAR_PTR_CAST "returnGraphWeights",
          call_returnGraphWeights, METH_VARARGS,
          CHAR_PTR_CAST "**returnGraphWeights** ()\n\nFills the entire image with white."        },
                        { CHAR_PTR_CAST "copyImage",
          call_copyImage, METH_VARARGS,
          CHAR_PTR_CAST "**copyImage** ()\n\nCopies an image."        },
              { NULL }
  };

                static PyObject* call_returnGraphWeights(PyObject* self, PyObject* args) {
            
      PyErr_Clear();
                                                                                double return_arg;
PyObject* return_pyarg;
                                          Image* self_arg;
PyObject* self_pyarg;
      
                                      if (PyArg_ParseTuple(args, CHAR_PTR_CAST "O:returnGraphWeights"
                        ,
             &self_pyarg                      ) <= 0)
           return 0;
               
              if (!is_ImageObject(self_pyarg)) {
          PyErr_SetString(PyExc_TypeError, "Argument 'self' must be an image");
          return 0;
        }
        self_arg = ((Image*)((RectObject*)self_pyarg)->m_x);
        image_get_fv(self_pyarg, &self_arg->features, &self_arg->features_len);
              
              try {
                      switch(get_image_combination(self_pyarg)) {
case ONEBITIMAGEVIEW:
return_arg = returnGraphWeights(*((OneBitImageView*)self_arg));
break;
case CC:
return_arg = returnGraphWeights(*((Cc*)self_arg));
break;
case ONEBITRLEIMAGEVIEW:
return_arg = returnGraphWeights(*((OneBitRleImageView*)self_arg));
break;
case RLECC:
return_arg = returnGraphWeights(*((RleCc*)self_arg));
break;
case MLCC:
return_arg = returnGraphWeights(*((MlCc*)self_arg));
break;
default:
PyErr_Format(PyExc_TypeError,"The 'self' argument of 'returnGraphWeights' can not have pixel type '%s'. Acceptable values are ONEBIT, ONEBIT, ONEBIT, ONEBIT, and ONEBIT.", get_pixel_type_name(self_pyarg));
return 0;
}
                  } catch (std::exception& e) {
          PyErr_SetString(PyExc_RuntimeError, e.what());
          return 0;
        }
      
                                            return_pyarg = PyFloat_FromDouble((double)return_arg);            return return_pyarg;
                              }
                static PyObject* call_copyImage(PyObject* self, PyObject* args) {
            
      PyErr_Clear();
                                                                                Image* return_arg;
PyObject* return_pyarg;
                                          Image* self_arg;
PyObject* self_pyarg;
      
                                      if (PyArg_ParseTuple(args, CHAR_PTR_CAST "O:copyImage"
                        ,
             &self_pyarg                      ) <= 0)
           return 0;
               
              if (!is_ImageObject(self_pyarg)) {
          PyErr_SetString(PyExc_TypeError, "Argument 'self' must be an image");
          return 0;
        }
        self_arg = ((Image*)((RectObject*)self_pyarg)->m_x);
        image_get_fv(self_pyarg, &self_arg->features, &self_arg->features_len);
              
              try {
                      switch(get_image_combination(self_pyarg)) {
case ONEBITIMAGEVIEW:
return_arg = copyImage(*((OneBitImageView*)self_arg));
break;
case CC:
return_arg = copyImage(*((Cc*)self_arg));
break;
case ONEBITRLEIMAGEVIEW:
return_arg = copyImage(*((OneBitRleImageView*)self_arg));
break;
case RLECC:
return_arg = copyImage(*((RleCc*)self_arg));
break;
case MLCC:
return_arg = copyImage(*((MlCc*)self_arg));
break;
default:
PyErr_Format(PyExc_TypeError,"The 'self' argument of 'copyImage' can not have pixel type '%s'. Acceptable values are ONEBIT, ONEBIT, ONEBIT, ONEBIT, and ONEBIT.", get_pixel_type_name(self_pyarg));
return 0;
}
                  } catch (std::exception& e) {
          PyErr_SetString(PyExc_RuntimeError, e.what());
          return 0;
        }
      
                                            if (return_arg== NULL) {
              if (PyErr_Occurred() == NULL) {
                Py_INCREF(Py_None);
                return Py_None;
               } else
                return NULL;
            } else {
              return_pyarg = create_ImageObject(return_arg);              return return_pyarg;
            }
                              }
      
  DL_EXPORT(void) init_stable_path_staff_detection(void) {
    Py_InitModule(CHAR_PTR_CAST "_stable_path_staff_detection", _stable_path_staff_detection_methods);
  }
  

