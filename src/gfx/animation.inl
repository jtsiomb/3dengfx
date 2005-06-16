/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

///////////////// Keyframes ////////////////

inline bool Keyframe::operator ==(const Keyframe &key) const {
	return time == key.time ? true : false;
}

inline bool Keyframe::operator <(const Keyframe &key) const {
	return time < key.time ? true : false;
}


// ------------- XFormNode -------------

inline Keyframe *XFormNode::get_nearest_key(unsigned long time) {
	return get_nearest_key(0, key_count > 0 ? key_count - 1 : 0, time);
}
inline const Keyframe *XFormNode::get_nearest_key(unsigned long time) const {
	return get_nearest_key(0, key_count > 0 ? key_count - 1 : 0, time);
}

inline const Keyframe *XFormNode::get_nearest_key(int start, int end, unsigned long time) const {
	return const_cast<XFormNode*>(this)->get_nearest_key(start, end, time);
}
