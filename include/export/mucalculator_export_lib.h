#ifndef MUCALCULATOR_EXPORT_LIB_H
#define MUCALCULATOR_EXPORT_LIB_H

#ifdef EXPORT_LIB_MUCALCULATOR
#define EXPORT_MUCALCULATOR Q_DECL_EXPORT
#else
#define EXPORT_MUCALCULATOR Q_DECL_IMPORT
#endif

#endif // MUCALCULATOR_EXPORT_LIB_H
