#ifndef MWIDGETS_EXPORT_LIB_H
#define MWIDGETS_EXPORT_LIB_H

#ifdef EXPORT_LIB_MWIDGETS
#define EXPORT_MWIDGETS Q_DECL_EXPORT
#else
#define EXPORT_MWIDGETS Q_DECL_IMPORT
#endif

#endif // MWIDGETS_EXPORT_LIB_H
