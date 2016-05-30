#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#  This file incorporates work covered by the following license notice:
# 
#    Modified May 2016 by Patrick Luby. NeoOffice is only distributed
#    under the GNU General Public License, Version 3 as allowed by Section 4
#    of the Apache License, Version 2.0.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#**************************************************************



$(eval $(call gb_AllLangResTarget_AllLangResTarget,vcl))

$(eval $(call gb_AllLangResTarget_set_reslocation,vcl,vcl))

$(eval $(call gb_AllLangResTarget_add_srs,vcl,\
	vcl/source/src \
))

$(eval $(call gb_SrsTarget_SrsTarget,vcl/source/src))

$(eval $(call gb_SrsTarget_set_include,vcl/source/src,\
        $$(INCLUDE) \
        -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_SrsTarget_add_files,vcl/source/src,\
    vcl/source/src/btntext.src \
    vcl/source/src/helptext.src \
    vcl/source/src/images.src \
    vcl/source/src/menu.src \
    vcl/source/src/print.src \
    vcl/source/src/stdtext.src \
    vcl/source/src/throbber.src \
    vcl/source/src/units.src \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_SrsTarget_add_files,vcl/source/src,\
    vcl/java/source/app/salinst.src \
))
endif


# vim: set noet sw=4 ts=4:
