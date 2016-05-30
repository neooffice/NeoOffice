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

ifeq ($(strip $(GUIBASE)),java)

$(eval $(call gb_Executable_Executable,checknativefont))

$(eval $(call gb_Executable_add_cobjects,checknativefont,\
    vcl/java/source/gdi/salchecknativefont \
))

$(eval $(call gb_Executable_set_ldflags,checknativefont,\
    $$(LDFLAGS) \
    -framework CoreFoundation \
    -framework CoreText \
))

endif

# vim: set ts=4 sw=4 et:
