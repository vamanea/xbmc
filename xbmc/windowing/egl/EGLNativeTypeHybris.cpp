/*
 *      Copyright (C) 2011-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#if defined(TARGET_HYBRIS)
#include <hwcomposerwindow/hwcomposer_window.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#endif

#include "system.h"
#include <EGL/egl.h>
#include "EGLNativeTypeHybris.h"
#include "utils/log.h"
#include "guilib/gui3d.h"

CEGLNativeTypeHybris::CEGLNativeTypeHybris()
{
}

CEGLNativeTypeHybris::~CEGLNativeTypeHybris()
{
} 

bool CEGLNativeTypeHybris::CheckCompatibility()
{
#if defined(TARGET_HYBRIS)
  if(hw_get_module(HWC_HARDWARE_MODULE_ID, (const hw_module_t **) &m_hwcModule))
  {
    return false;
  }

  if(hwc_open_1(m_hwcModule, &m_hwcDevicePtr))
  {
    return false;
  }

  return true;
#else
  return false;
#endif
}

void CEGLNativeTypeHybris::Initialize()
{
#if defined(TARGET_HYBRIS)
  m_hwcDevicePtr->blank(hwcDevicePtr, 0, 0);
#endif
}

void CEGLNativeTypeHybris::Destroy()
{
  return;
}

bool CEGLNativeTypeHybris::CreateNativeDisplay()
{
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeHybris::CreateNativeWindow()
{
#if defined(TARGET_HYBRIS)
  RESOLUTION_INFO res;
  if (!GetNativeResolution(&res))
    return false;
 
  m_nativeWindow = new (XBNativeWindowType)HWComposerNativeWindow(res->iWidth, res->iHeight, HAL_PIXEL_FORMAT_RGBA_8888);

  size_t size = sizeof(hwc_display_contents_1_t) + 2 * sizeof(hwc_layer_1_t);
  
  hwc_display_contents_1_t *list = (hwc_display_contents_1_t *) malloc(size);
  mList = (hwc_display_contents_1_t **) malloc(HWC_NUM_DISPLAY_TYPES * sizeof(hwc_display_contents_1_t *));
  const hwc_rect_t r = { 0, 0, res->iWidth, res->iHeight };

  int counter = 0;
  for (; counter < HWC_NUM_DISPLAY_TYPES; counter++)
    mList[counter] = list;

  hwc_layer_1_t *layer = &m_bufferList->hwLayers[0];
  memset(layer, 0, sizeof(hwc_layer_1_t));
  layer->compositionType = HWC_FRAMEBUFFER;
  layer->hints = 0;
  layer->flags = 0;
  layer->handle = 0;
  layer->transform = 0;
  layer->blending = HWC_BLENDING_NONE;
  layer->sourceCrop = r;
  layer->displayFrame = r;
  layer->visibleRegionScreen.numRects = 1;
  layer->visibleRegionScreen.rects = &layer->displayFrame;
  layer->acquireFenceFd = -1;
  layer->releaseFenceFd = -1;
  layer = &list->hwLayers[1];
  memset(layer, 0, sizeof(hwc_layer_1_t));
  layer->compositionType = HWC_FRAMEBUFFER_TARGET;
  layer->hints = 0;
  layer->flags = 0;
  layer->handle = 0;
  layer->transform = 0;
  layer->blending = HWC_BLENDING_NONE;
  layer->sourceCrop = r;
  layer->displayFrame = r;
  layer->visibleRegionScreen.numRects = 1;
  layer->visibleRegionScreen.rects = &layer->displayFrame;
  layer->acquireFenceFd = -1;
  layer->releaseFenceFd = -1;

  list->retireFenceFd = -1;
  list->flags = HWC_GEOMETRY_CHANGED;
  list->numHwLayers = 2;

  return (m_nativeWindow != NULL);
#else
  return false;
#endif
}

bool CEGLNativeTypeHybris::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;

  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;

  return true;
}

bool CEGLNativeTypeHybris::GetNativeWindow(XBNativeWindowType **nativeWindow) const
{
  if (!nativeWindow || !m_nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;

  return true;
}

bool CEGLNativeTypeHybris::DestroyNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeHybris::DestroyNativeWindow()
{
  m_nativeWindow = NULL;
  return true;
}

bool CEGLNativeTypeHybris::GetNativeResolution(RESOLUTION_INFO *res) const
{
#if defined(TARGET_HYBRIS)
  uint32_t configs[5];
  size_t numConfigs = 5;

  err = hwcDevicePtr->getDisplayConfigs(hwcDevicePtr, 0, configs, &numConfigs);
  if (err) {
      CLog::Log(LOGERROR, "getDisplayConfigs failed!");
      return false;
  }
  int32_t attr_values[2];
  uint32_t attributes[] = { HWC_DISPLAY_WIDTH, HWC_DISPLAY_HEIGHT, HWC_DISPLAY_NO_ATTRIBUTE };

  m_hwcDevicePtr->getDisplayAttributes(m_hwcDevicePtr, 0, configs[0], attributes, attr_values);

  res->iWidth = attr_values[0];
  res->iHeight = attr_values[1];

  res->fRefreshRate  = 60;
  res->dwFlags       = D3DPRESENTFLAG_PROGRESSIVE;
  res->iScreen       = 0;
  res->bFullScreen   = true;
  res->iSubtitles    = (int)(0.965 * res->iHeight);
  res->fPixelRatio   = 1.0f;
  res->iScreenWidth  = res->iWidth;
  res->iScreenHeight = res->iHeight;
  res->strMode.Format("%dx%d @ %.2f%s - Full Screen", res->iScreenWidth, res->iScreenHeight, res->fRefreshRate,
  res->dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
  CLog::Log(LOGNOTICE,"Current resolution: %s\n",res->strMode.c_str());
  return true;
#endif
}

bool CEGLNativeTypeHybris::SetNativeResolution(const RESOLUTION_INFO &res)
{
  return false;
}

bool CEGLNativeTypeHybris::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  RESOLUTION_INFO res;
  bool ret = false;
  ret = GetNativeResolution(&res);
  if (ret && res.iWidth > 1 && res.iHeight > 1)
  {
    resolutions.push_back(res);
    return true;
  }
  return false;
}

bool CEGLNativeTypeHybris::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  return false;
}

bool CEGLNativeTypeHybris::ShowWindow(bool show)
{
#if defined(TARGET_HYBRIS)
  HWComposerNativeWindow* window = (HWComposerNativeWindow*)m_nativeWindow;
  HWComposerNativeWindowBuffer *front;
  window->lockFrontBuffer(&front);

  m_bufferList[0]->hwLayers[1].handle = front->handle;
  m_bufferList[0]->hwLayers[0].handle = NULL;
  m_bufferList[0]->hwLayers[0].flags = HWC_SKIP_LAYER;

  oldretire = m_bufferList[0]->retireFenceFd;
  oldrelease = m_bufferList[0]->hwLayers[1].releaseFenceFd;
  oldrelease2 = m_bufferList[0]->hwLayers[0].releaseFenceFd;

  int err = m_hwcDevicePtr->prepare(hwcDevicePtr, HWC_NUM_DISPLAY_TYPES, m_bufferList);
  if(err) {
    printf("prepare() failed!");
  }

  err = m_hwcDevicePtr->set(m_hwcDevicePtr, HWC_NUM_DISPLAY_TYPES, m_bufferList);

  window->unlockFrontBuffer(front);
  if (oldrelease != -1)
  {
    sync_wait(oldrelease, -1);
    close(oldrelease);
  }
  if (oldrelease2 != -1)
  {
    sync_wait(oldrelease2, -1);
    close(oldrelease2);
  }
  
  if (oldretire != -1)
  {
    sync_wait(oldretire, -1);
    close(oldretire);
  }
#endif
  return true;
}
