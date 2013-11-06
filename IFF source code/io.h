/* @(#) io.h 96/04/19 1.3 */

#ifndef	__IO_H
#define	__IO_H


/*****************************************************************************/


#include <kernel/types.h>
#include <misc/iff.h>


/*****************************************************************************/


#define IFFOpen(iff,k,m)  (* iff->iff_IOFuncs->io_Open)(&iff->iff_IOContext,k,m)
#define IFFClose(iff)     (* iff->iff_IOFuncs->io_Close)(iff->iff_IOContext)
#define IFFRead(iff,b,s)  (* iff->iff_IOFuncs->io_Read)(iff->iff_IOContext,b,s)
#define IFFWrite(iff,b,s) (* iff->iff_IOFuncs->io_Write)(iff->iff_IOContext,b,s)
#define IFFSeek(iff,p)    (* iff->iff_IOFuncs->io_Seek)(iff->iff_IOContext,p)


/*****************************************************************************/


extern const IFFIOFuncs fileFuncs;


/*****************************************************************************/


#endif /* __IO_H */
