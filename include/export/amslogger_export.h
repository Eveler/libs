#ifndef AMSLOGGER_EXPORT_H
#define AMSLOGGER_EXPORT_H

#ifdef AMSLOGGER_BUILD
#define AMSLOGGER_EXPORT Q_DECL_EXPORT
#else
#define AMSLOGGER_EXPORT Q_DECL_IMPORT
#endif

#endif // AMSLOGGER_EXPORT_H
