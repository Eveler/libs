#ifndef MDATASET_EXPORT_LIB_H
#define MDATASET_EXPORT_LIB_H

#ifdef EXPORT_LIB_MDATASET
#define EXPORT_MDATASET Q_DECL_EXPORT
#else
#define EXPORT_MDATASET Q_DECL_IMPORT
#endif

#endif // MDATASET_EXPORT_LIB_H
