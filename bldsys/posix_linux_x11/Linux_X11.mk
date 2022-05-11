#
# Copyright (C) YuqiaoZhang(HanetakaChou)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

LOCAL_PATH:= $(call my-dir)

# vkcube

include $(CLEAR_VARS)

LOCAL_MODULE := vkcube

LOCAL_SRC_FILES := \
	$(abspath $(LOCAL_PATH)/../..)/main.cpp \
	$(abspath $(LOCAL_PATH)/../..)/generated.cpp \
	$(abspath $(LOCAL_PATH)/../../utils)/TextureLoader.cpp \
	$(abspath $(LOCAL_PATH)/../../utils)/DDS/TextureLoader_DDS.cpp \
	$(abspath $(LOCAL_PATH)/../../utils)/PVR/TextureLoader_PVR.cpp \
	$(abspath $(LOCAL_PATH)/../../utils)/VK/TextureLoader_VK.cpp \
	$(abspath $(LOCAL_PATH)/../../utils)/VK/StagingBuffer.cpp

LOCAL_CFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
LOCAL_CFLAGS += -Werror=return-type
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -Wall

LOCAL_LDFLAGS += -Wl,--enable-new-dtags # the linker can't recognize the old dtags
LOCAL_LDFLAGS += -Wl,-rpath,XORIGIN # chrpath can only make path shorter
LOCAL_LDFLAGS += -Wl,--version-script,$(abspath $(LOCAL_PATH))/executable.def

LOCAL_C_INCLUDES += $(abspath $(LOCAL_PATH)/../../DirectXMath)/Inc 

LOCAL_LDFLAGS += -lvulkan
LOCAL_LDFLAGS += -lxcb

include $(BUILD_EXECUTABLE)