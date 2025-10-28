/*
  Minimap QtCreator plugin.

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not see
  http://www.gnu.org/licenses/lgpl-2.1.html.

  Copyright (c) 2017, emJay Software Consulting AB, See AUTHORS for details.
*/

//! @file minimapconstants.h
//!
//! Constans used throughout the Minimap plugin

#pragma once

namespace Minimap {
namespace Constants {
const char MINIMAP_ID[] = "Minimap.Minimap";
const char MINIMAP_SETTINGS[] = "Z.MinimapSettings";
const char MINIMAP_STYLE_OBJECT_PROPERTY[] = "Minimap.StyleObject";
const int MINIMAP_WIDTH_DEFAULT = 80;
const int MINIMAP_EXTRA_AREA_WIDTH = 7;
const int MINIMAP_MAX_LINE_COUNT_DEFAULT = 8000;
const int MINIMAP_ALPHA_DEFAULT = 32;
const bool MINIMAP_CENTER_ON_CLICK_DEFAULT = true;
const bool MINIMAP_SHOW_LINE_TOOLTIP_DEFAULT = true;
const int MINIMAP_PIXELS_PER_LINE_DEFAULT = 2;
} // namespace Constants
} // namespace Minimap
