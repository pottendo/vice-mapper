/*  -*-c-*-
 * File:		VmTileImg.h
 * Date:		Mon Jul 27 15:34:39 2020
 * Author:		pottendo (pottendo)
 *  
 * Abstract:
 *     image tile class
 * 
 * Modifications:
 * $Log$
 */

#ifndef __VmTileImg_h__
#define __VmTileImg_h__

#include "VmTile.h"

class VmTileImg : public VmTile {
  protected:
    virtual void on_draw_specific(const Cairo::RefPtr<Cairo::Context>& cr) override {};
  public:
    VmTileImg(VmMap &m, const char *fn, int x = -1, int y = -1);
    ~VmTileImg() {};
};

#endif /* __VmTileImg_h__ */
