<?xml version="1.0" encoding="UTF-8"?>

<!--
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   | This is the transformation style sheet for Help files            |
   | (main_tranform.xsl).                                             |
   |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
   | Copyright © 2002 Sun Microsystems, Inc. All rights reserved.     |
   | Use of this product is subject to license terms.                 |
   |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
   | FPE                                                              |
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   | You should not edit this file unless you know what you're doing! |
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   | Revision History                                                 |
   | ================                                                 |
   |                                                                  |
   |  27-Jul-01: Evaluation of $Program replaced by declaration       |
   |             of $Prog for the same purpose                        |
   |  09-Aug-01: Added evaluation of th in "How to get"-popups.       | 
   |             Changed evaluation of icon/name tables within that.  |
   |  09-Aug-01: Added special formatting of "Related" header         |
   |             (styleclass "reltop")                                |
   |  09-Aug-01: Added variable for column width of first columns of  |
   |             tables with icons                                    |
   |  14-Aug-01: Fixed Bug: Switches were not evaluated in document   |
   |             title h1                                             |
   |  14-Aug-01: Fixed Bug: System switches were falsely evaluated.   |
   |             Changed the way of switch evaluation, now using modes|
   |  17-AUG-01: Fixed Bug: switch as direct child of embedded were   |
   |             not evaluated                                        |
   |  22-Aug-01: Fixed: Axis PRECEDING no longer used (wasn't         |
   |             supported by processor)                              |
   |  22-Aug-01: Now empty Howtoget links are ignored                 |
   |  27-Aug-01: Now the image size is evaluated (from cm to pixels)  |
   |             and included in the img tags.                        |
   |  28-Aug-01: Embedded Paras now check for a <p> ancestor          |
   |             instead of a parent.                                 |
   |  30-Aug-01: Added recursive evaluation of embedded text          |
   |  02-Oct-01: ABI: Added parameter cs for use in css url           |
   |  10-Oct-01: Added handling for text:tab-stop and text:s          |
   |  10-Oct-01: Added handling for source file admin information     |
   |  15-Oct-01: Fixed Bug: fix URL vnd.sun.star.help for internal URL| 
   |             calls (embedded + popup-cut), $sm for external calls |
   |  19-Oct-01: Added handling of product variable parameters        |
   |  25-Oct-01: Removed insertion of hr before h2                    |
   |  25-Oct-01: Added language handling                              |
   |  15-Nov-01: Fixed Bug: embedded productname tags were not        |
   |             replaced                                             |
   |  15-Nov-01: Fixed Bug: embedde ol/ul were not evaluated due      |
   |             to a typo                                            |
   |  10-Dec-01: Changed transcription of "Head"s. Now they are       |
   |             transformed to <h>s                                  |
   |  10-Dec-01: Added OOo-switch for Product                         |
   |  26-Mar-02: Added autosetting of name for hyperlink tags         |
   |             (Accessibility issue)                                |
   |  16-May-02: Fixed autonaming of links                            |
   |  16-May-02: Added alt tag for images                             |
   |  16-May-02: Changed size calculation for images                  |
   |  27-Jun-02: Fixed bug, no evaluation of switches inside certain  |
   |             popups                                               |
   |  11-Oct-02: For accessibility reasons: Removed formatting topic  |
   |             title inside table                                   |
   |  27-Jan-03: Fixed bug for PRODUCT switch                         |
   |  27-Jan-03: Added <help:paragraphinfo/> template                 |
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-->

<xsl:stylesheet version="1.0" 
  xmlns:nu="http://www.jclark.com/xt/java/com.sun.xmlsearch.tree.NodeUtils" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
  xmlns:office="http://openoffice.org/2000/office" 
  xmlns:style="http://openoffice.org/2000/style"  
  xmlns:text="http://openoffice.org/2000/text"  
  xmlns:table="http://openoffice.org/2000/table"  
  xmlns:draw="http://openoffice.org/2000/drawing" 
  xmlns:fo="http://www.w3.org/1999/XSL/Format"  
  xmlns:xlink="http://www.w3.org/1999/xlink" 
  xmlns:dc="http://purl.org/dc/elements/1.1/"   
  xmlns:meta="http://openoffice.org/2000/meta" 
  xmlns:number="http://openoffice.org/2000/datastyle" 
  xmlns:svg="http://www.w3.org/2000/svg" 
  xmlns:chart="http://openoffice.org/2000/chart" 
  xmlns:help="http://openoffice.org/2000/help">
  
  <!-- Generate html code -->
  <xsl:output method="html"/>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | STYLESHEET PARAMETER DEFINITION                                  |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | These are used to define the parameters handed over to the       |
  | stylesheet processor. They are used to build the help URLs       |
  | and for application and product/name evaluation.                 |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

	<!-- Unused parameter of mysterious origin ?-->
  <xsl:param name="Device" select="'WARP'"/>
  
  <!-- Parameters used to construct the URL prefix -->
  <xsl:param name="hp" select="''"/>
  <xsl:param name="sm" select="'vnd.sun.star.help://'"/>
  <xsl:param name="qm" select="'?'"/>
  <xsl:param name="es" select="'='"/>
  <xsl:param name="am" select="'&amp;'"/>
  <xsl:param name="cl" select="':'"/>
  <xsl:param name="sl" select="'/'"/>
  <xsl:param name="hm" select="'#'"/>
  <xsl:param name="cs" select="''"/>
  
  <!-- Parameter used to identify the current application database -->
  <xsl:param name="Database" select="'swriter'"/>

  <!-- Parameter used to identify the current language -->
  <xsl:param name="Language" select="'de'"/>
  
  <!-- Parameter to identify the OS/platform used -->
  <xsl:param name="System" select="'WIN'"/>

	<!-- Parameters used to output help error information -->
  <xsl:param name="Id" select="'0000'"/>
  <xsl:param name="Path" select="'Path'"/>

	<!-- Parameters used to assign product names -->
  <xsl:param name="productname" select="'Office'"/>
  <xsl:param name="productversion" select="''"/>
  <xsl:param name="vendorname" select="'Webtop'"/>
  <xsl:param name="vendorversion" select="''"/>
  <xsl:param name="vendorshort" select="'Webtop'"/>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | STYLESHEET VARIABLE DEFINITION                                   |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | These are used for repeated tasks in this stylesheet and only    |
  | defined once here.                                               |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

  <!-- 
		Variable to identify the application used for evaluation of program switches
		This one is derived from the database name	
	-->
  <xsl:variable name="Program">
    <xsl:choose>
      <xsl:when test="$Database='swriter'">WRITER</xsl:when>
      <xsl:when test="$Database='scalc'">CALC</xsl:when>
      <xsl:when test="$Database='sdraw'">DRAW</xsl:when>
      <xsl:when test="$Database='simpress'">IMPRESS</xsl:when>
      <xsl:when test="$Database='sbasic'">BASIC</xsl:when>
      <xsl:when test="$Database='smath'">MATH</xsl:when>
      <xsl:when test="$Database='schart'">CHART</xsl:when>
      <xsl:when test="$Database='portal'">PORTAL</xsl:when>
      <xsl:otherwise>NONE</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  
	<!-- 
		Variable to define the language of the help document which may be different
		from the one defined in the Language *parameter*! It is evaluated from a
		meta tag in the xml source
	-->
  <xsl:variable name="lang">
    <xsl:for-each select="/html/head/meta">
      <xsl:if test="@name='language'">
        <xsl:value-of select="@content"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  
	<!-- Variables for formatting purposes. "onepixel" is the image number of a transparent 1-pixel image -->
  <xsl:variable name="colwidth_1">25</xsl:variable>
  <xsl:variable name="spacewidth">5</xsl:variable>
  <xsl:variable name="tabwidth">15</xsl:variable>
  <xsl:variable name="onepixel">67433</xsl:variable>

	<!-- Variable for distinguishing the Open Source distribution -->
  <xsl:variable name="Prod">
    <xsl:choose>
      <xsl:when test="$productname='StarOffice'">Commercial</xsl:when>
      <xsl:when test="$productname='StarSuite'">Commercial</xsl:when>
      <xsl:otherwise>OpenSource</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | BASIC PROCESSING OF ELEMENTS                                     |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

  <!-- Copy through of all elements including their attributes -->
  <xsl:template match="*|@*|comment()|processing-instruction()|text()">
    <xsl:copy>
      <xsl:apply-templates select="*|@*|comment()|processing-instruction()|text()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="help:*">
    <xsl:element name="{name()}">
      <xsl:apply-templates select="*|@*|comment()|processing-instruction()|text()"/>
    </xsl:element>
  </xsl:template>

  <!-- The language of the document body is set here as an attribute of the body tag -->
  <xsl:template match="body">
    <xsl:element name="body">
      <xsl:attribute name="lang"><xsl:value-of select="$lang"/></xsl:attribute>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>


  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF SPECIFIC HELP ELEMENTS                             |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

  <xsl:template match="help:to-be-embedded">
    <xsl:apply-templates/>
  </xsl:template>
  
	<xsl:template match="help:to-popup">
    <xsl:apply-templates/>
  </xsl:template>
  
	<!-- Text for active help must only show in the content if set to visible -->
  <xsl:template match="help:help-text">
    <xsl:if test="@value='visible'">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

	<xsl:template match="help:paragraphinfo">
	</xsl:template>

  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | CONDITIONAL PROCESSING                                           |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	| There are program, system and product switches throughout the    |
	| help enabling or disabling content depending on their context    |
	| which is described using the parameters/variables defined above. |
	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

	<!-- Switches inside normal text -->
  <xsl:template match="help:switch">
    <xsl:choose>
      <xsl:when test="@select='Program'">
        <xsl:apply-templates mode="switch_prog"/>
      </xsl:when>
      <xsl:when test="@select='System'">
        <xsl:apply-templates mode="switch_sys"/>
      </xsl:when>
      <xsl:when test="@select='Product'">
        <xsl:apply-templates mode="switch_prod"/>
      </xsl:when>
      <xsl:when test="@select='PRODUCT'">
        <xsl:apply-templates mode="switch_prod"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

	<!-- Switches inside popups -->
  <xsl:template match="help:switch" mode="popup">
    <xsl:choose>
      <xsl:when test="@select='Program'">
        <xsl:apply-templates mode="switch_prog"/>
      </xsl:when>
      <xsl:when test="@select='System'">
        <xsl:apply-templates mode="switch_sys"/>
      </xsl:when>
      <xsl:when test="@select='Product'">
        <xsl:apply-templates mode="switch_prod"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>



	<!-- Switches inside text which is embedded from elsewhere -->
  <xsl:template match="help:switch" mode="embedded">
    <xsl:choose>
      <xsl:when test="@select='Program'">
        <xsl:apply-templates mode="switch_prog"/>
      </xsl:when>
      <xsl:when test="@select='System'">
        <xsl:apply-templates mode="switch_sys"/>
      </xsl:when>
      <xsl:when test="@select='Product'">
        <xsl:apply-templates mode="switch_prod"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

	
  <xsl:template match="help:case" mode="switch_prog">
    <xsl:if test="@select=$Program">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

	<xsl:template match="help:default" mode="switch_prog">
    <xsl:if test="not(../help:case[@select=$Program])">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="help:case" mode="switch_sys">
    <xsl:if test="@select=$System">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="help:default" mode="switch_sys">
    <xsl:if test="not(../help:case[@select=$System])">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="help:case" mode="switch_prod">
    <xsl:if test="@select=$Prod">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="help:default" mode="switch_prod">
    <xsl:if test="not(../help:case[@select=$Prod])">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>

	<!-- Switches inside the document title -->
  <xsl:template match="help:switch" mode="doctitle">
    <xsl:if test="@select='Program' or @select='System'">
      <xsl:apply-templates mode="doctitle"/>
    </xsl:if>
  </xsl:template>
  <xsl:template match="help:case" mode="doctitle">
    <xsl:if test="@select=$Program or @select=$System">
      <xsl:apply-templates mode="doctitle"/>
    </xsl:if>
  </xsl:template>
  <xsl:template match="help:default" mode="doctitle">
    <xsl:if test="../../help:switch[@select='System'] and not(../help:case[@select=$System])">
      <xsl:apply-templates mode="doctitle"/>
    </xsl:if>
    <xsl:if test="../../help:switch[@select='Program'] and not(../help:case[@select=$Program])">
      <xsl:apply-templates mode="doctitle"/>
    </xsl:if>
  </xsl:template>

  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF PARAGRAPHS AND HEADINGS                            |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | This one is quite cascaded to take account of the different para |
  | contexts. This includes headers as in the xml source files these |
  | are only paragraphs with a special style class, viz "Head"       |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

	<!-- Normal Paragraphs --> 
  <xsl:template match="p">

    <!-- Disregard empty paragraphs -->
    <xsl:choose>
      <xsl:when test=".='' and count(child::*)=0"/>
      <xsl:otherwise>

        <!-- Process all headings -->
        <xsl:choose>
          <xsl:when test="substring(@class,1,4)='Head'">

            <!-- Head 1 should only be the document title at the top and is treated differently -->
            <!-- removed table as formatting help -->
            <xsl:choose>
              <xsl:when test="@class='Head1'">
                      <xsl:element name="h1">
                        <xsl:apply-templates mode="doctitle"/>
                      </xsl:element>
                <xsl:element name="br"/>
              </xsl:when>

						<!-- All other Heads are converted to HTML headings -->
              <xsl:when test="@class='Head2'">
                <xsl:element name="h2">
                  <xsl:apply-templates/>
                </xsl:element>
              </xsl:when>

              <xsl:when test="@class='Head3'">
                <xsl:element name="h3">
                  <xsl:apply-templates/>
                </xsl:element>
              </xsl:when>

              <xsl:when test="@class='Head4'">
                <xsl:element name="h4">
                  <xsl:apply-templates/>
                </xsl:element>
              </xsl:when>

              <xsl:when test="@class='Head5'">
                <xsl:element name="h5">
                  <xsl:apply-templates/>
                </xsl:element>
              </xsl:when>

              <xsl:when test="@class='Head6'">
                <xsl:element name="h6">
                  <xsl:apply-templates/>
                </xsl:element>
              </xsl:when>

            </xsl:choose>

          </xsl:when>
          
				<!-- 
            All Non-Header paragraphs are converted into HTML p with corresponding style class assigned.
            Valid style classes are Paragraph, ParaList, TextInTable, TableHead, PropText
          -->

          <xsl:when test="@class='Paragraph' 
                       or @class='ParaList' 
                       or @class='TextInTable' 
                       or @class='TableHead' 
                       or @class='PropText'">
            <xsl:element name="p">
              <xsl:choose>
                <!-- By special request, turn the "Related Topics" header into an own style class -->
                <xsl:when test="child::help:embedded[@Eid='related']">
                  <xsl:attribute name="class">reltop</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:attribute name="class"><xsl:value-of select="@class"/></xsl:attribute>
                </xsl:otherwise>
                </xsl:choose>
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>

          <!-- all unknown paragraph styles are just converted into the standard "paragraph" style -->
          <xsl:otherwise>
            <xsl:element name="p">
              <xsl:attribute name="class">Paragraph</xsl:attribute>
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:otherwise>

        </xsl:choose>

      </xsl:otherwise> <!-- non-empty paras-->
    </xsl:choose>
  </xsl:template>

  <!-- Paragraphs inside text which is embedded from somewhere else -->
  <xsl:template match="p" mode="embedded">
  
    <!-- Disregard empty paragraphs -->
    <xsl:choose>
      <xsl:when test=".='' and count(child::*)=0"/>
      <xsl:otherwise>
        <xsl:choose>
          
          <xsl:when test="@class='Head1'">
            <xsl:element name="h2">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
          
          <xsl:when test="@class='Head2'">
            <xsl:element name="h2">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
      
          <xsl:when test="@class='Head3'">
            <xsl:element name="h3">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
          
          <xsl:when test="@class='Head4'">
            <xsl:element name="h4">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
          
          <xsl:when test="@class='Head5'">
            <xsl:element name="h5">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
          
          <xsl:when test="@class='Head6'">
            <xsl:element name="h6">
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
      
      		<!-- 
            All Non-Header paragraphs are converted into HTML p with corresponding style class assigned.
            Valid style classes are Paragraph, ParaList, TextInTable, TableHead, PropText
          -->
          
          <xsl:when test="@class='Paragraph' 
                       or @class='ParaList' 
                       or @class='TextInTable' 
                       or @class='TableHead' 
                       or @class='PropText'">
            <xsl:element name="p">
              <xsl:attribute name="class"><xsl:value-of select="@class"/></xsl:attribute>
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:when>
        
          <!-- all unknown paragraph styles are just converted into the standard "paragraph" style -->
          <xsl:otherwise>
            <xsl:element name="p">
              <xsl:attribute name="class">Paragraph</xsl:attribute>
              <xsl:apply-templates/>
            </xsl:element>
          </xsl:otherwise>
    
        </xsl:choose>
      </xsl:otherwise> <!-- non-empty paragraphs -->
    </xsl:choose>
  </xsl:template>

  <!-- Paragraphs inside popup text (mainly the information of how to get a function) -->
  <xsl:template match="p" mode="popup">
    <xsl:if test="(. != '') or (count(child::*)>0)">
      <xsl:element name="p">
        <xsl:attribute name="class">howtogetpara</xsl:attribute>
        <xsl:apply-templates/>
      </xsl:element>
    </xsl:if>
  </xsl:template>

  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF LISTS                                              |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->

  <!-- Numbered lists in normal text -->
  <xsl:template match="ol">
    <xsl:element name="ol">	
    	<xsl:if test="@start">
    		<xsl:attribute name="start"><xsl:value-of select="@start"/></xsl:attribute>
    	</xsl:if>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  <!-- Numbered lists in text embedded from somewhere else -->
  <xsl:template match="ol" mode="embedded">
    <xsl:element name="ol">
    	<xsl:if test="@start">
    		<xsl:attribute name="start"><xsl:value-of select="@start"/></xsl:attribute>
    	</xsl:if>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  <!-- Bulleted lists in normal text -->
  <xsl:template match="ul">
    <xsl:element name="ul">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  <!-- Bulleted lists in text embedded from somewhere else -->
  <xsl:template match="ul" mode="embedded">
    <xsl:element name="ul">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  <!-- List items in any context -->
  <xsl:template match="li">
    <xsl:element name="li">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF CHARACTER CLASSES                                  |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Process character formatting -->
  <xsl:template match="span">
    <!-- Disregard spans with table automatic formats, starting with "Tabelle" -->
    <xsl:choose>
      <xsl:when test="substring(@class,1,7)='Tabelle'">
        <xsl:apply-templates/>
      </xsl:when>
      <xsl:otherwise>
        <!-- insert emphasized span -->
        <xsl:element name="span">
          <xsl:attribute name="class">emph</xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="span" mode="embedded">
    <!-- Disregard spans with table automatic formats, starting with "Tabelle" -->
    <xsl:choose>
      <xsl:when test="substring(@class,1,7)='Tabelle'">
        <xsl:apply-templates/>
      </xsl:when>
      <xsl:otherwise>
        <!-- insert emphasized span -->
        <xsl:element name="span">
          <xsl:attribute name="class">emph</xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="span" mode="popup">
    <!-- Disregard spans with table automatic formats, starting with "Tabelle" -->
    <xsl:choose>
      <xsl:when test="substring(@class,1,7)='Tabelle'">
        <xsl:apply-templates/>
      </xsl:when>
      <xsl:otherwise>
        <!-- insert emphasized span -->
        <xsl:element name="span">
          <xsl:attribute name="class">emph</xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF TABLES                                             |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Tables should be preceded and followed by a line break -->
  <xsl:template match="table">
    <xsl:element name="br"/>
    <xsl:element name="table">
      <xsl:if test="@border>0">
        <xsl:attribute name="border">1</xsl:attribute>
        <xsl:attribute name="class">tablebg</xsl:attribute>
      </xsl:if>
      <xsl:attribute name="cellspacing">0</xsl:attribute>
      <xsl:attribute name="cellpadding">2</xsl:attribute>
      <xsl:apply-templates/>
    </xsl:element>
    <xsl:element name="br"/>
  </xsl:template>
  <xsl:template match="table" mode="embedded">
    <xsl:element name="br"/>
    <xsl:element name="table">
      <xsl:if test="@border>0">
        <xsl:attribute name="border">1</xsl:attribute>
        <xsl:attribute name="class">tablebg</xsl:attribute>
      </xsl:if>
      <xsl:attribute name="cellspacing">0</xsl:attribute>
      <xsl:attribute name="cellpadding">2</xsl:attribute>
      <xsl:apply-templates/>
    </xsl:element>
    <xsl:element name="br"/>
  </xsl:template>
  <xsl:template match="table" mode="popup">
    <xsl:element name="table">
      <xsl:attribute name="border">0</xsl:attribute>
      <xsl:attribute name="cellspacing">0</xsl:attribute>
      <xsl:attribute name="cellpadding">2</xsl:attribute>
      <xsl:apply-templates mode="popup"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="tr">
    <xsl:element name="tr">
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="tr" mode="popup">
    <xsl:element name="tr">
      <xsl:apply-templates mode="popup"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="th">
    <!-- no th for borderless tables - 24/07/01-->
    <xsl:choose>
      <xsl:when test="../../table[@border>0]">
        <xsl:element name="th">
          <xsl:attribute name="align">left</xsl:attribute>
          <xsl:attribute name="valign">top</xsl:attribute>
          <xsl:attribute name="class">tableheadbg</xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:when>
      <xsl:otherwise>
        <xsl:element name="td">
          <xsl:attribute name="align">left</xsl:attribute>
          <xsl:attribute name="valign">top</xsl:attribute>
          <xsl:attribute name="class">tabledatabg</xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="td" mode="popup">
    <xsl:element name="td">
      <xsl:if test="descendant::draw:image">
        <xsl:attribute name="width"><xsl:value-of select="$colwidth_1"/></xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="popup"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="th" mode="popup">
    <xsl:element name="td">
      <xsl:if test="descendant::draw:image">
        <xsl:attribute name="width"><xsl:value-of select="$colwidth_1"/></xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="popup"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="td">
    <xsl:element name="td">
      <xsl:attribute name="align">left</xsl:attribute>
      <xsl:attribute name="valign">top</xsl:attribute>
      <xsl:attribute name="class">tabledatabg</xsl:attribute>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF TEXT EMBEDDED FROM SOMEHWERE ELSE                  |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  
  <!-- Process embedded help text -->
  <xsl:template match="help:embedded">
    <xsl:variable name="Link" select="concat('vnd.sun.star.help://',$Database,'/',@Id,'?Language=',$Language,'&amp;System=',$System)"/>
    <xsl:variable name="EmbeddedDoc" select="document($Link)"/>
    <xsl:variable name="Docpart" select="@Eid"/>
    <xsl:choose>
      <!-- prevent embedded paragraphs without p tag -->
      <xsl:when test="not(ancestor::p)">
        <xsl:element name="p">
          <xsl:attribute name="class">Paragraph</xsl:attribute>
          <xsl:apply-templates select="$EmbeddedDoc//help:to-be-embedded[@Eid=$Docpart]" mode="embedded"/>
        </xsl:element>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$EmbeddedDoc//help:to-be-embedded[@Eid=$Docpart]" mode="embedded"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <!-- Process embedded help text -->
  <xsl:template match="help:embedded" mode="embedded">
    <xsl:variable name="Link" select="concat($sm,$Database,'/',@Id,'?Language=',$Language,'&amp;System=',$System)"/>
    <xsl:variable name="EmbeddedDoc" select="document($Link)"/>
    <xsl:variable name="Docpart" select="@Eid"/>
    <xsl:choose>
      <!-- prevent embedded paragraphs without p tag -->
      <xsl:when test="not(ancestor::p)">
        <xsl:element name="p">
          <xsl:attribute name="class">Paragraph</xsl:attribute>
          <xsl:apply-templates select="$EmbeddedDoc//help:to-be-embedded[@Eid=$Docpart]" mode="embedded"/>
        </xsl:element>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$EmbeddedDoc//help:to-be-embedded[@Eid=$Docpart]" mode="embedded"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF POPUP TEXT                                         |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Process Popup help text -->
  <xsl:template match="help:popup-cut">
    <xsl:variable name="ref" select="concat('vnd.sun.star.help://',$Database,'/',@Id,'?Language=',$Language,'&amp;System=',$System)"/>
    <xsl:variable name="popupdoc" select="document($ref)"/>
    <xsl:variable name="part" select="@Eid"/>
    <xsl:apply-templates select="$popupdoc//help:to-popup[@Eid=$part]"/>
    <!--
        <xsl:element name="hr">
          <xsl:attribute name="width">50%</xsl:attribute>
          <xsl:attribute name="align">center</xsl:attribute>
          <xsl:attribute name="size">1</xsl:attribute>
        </xsl:element>
//-->
  </xsl:template>
  
  <!-- Process help popups -->
  <!-- Workaround for "How to get this function" popup is to load it in a small table into the document -->
  <xsl:template match="help:popup">
    <xsl:choose>
      <xsl:when test="child::help:embedded[@Eid='wie']">
        <xsl:variable name="Link" select="concat('vnd.sun.star.help://',$Database,'/',@Id,'?Language=',$Language,'&amp;System=',$System)"/>
        <xsl:variable name="EmbeddedDoc" select="document($Link)"/>
        <xsl:variable name="Docpart" select="@Eid"/>
        <xsl:if test="$EmbeddedDoc//help:to-popup[@Eid=$Docpart]!=''">
          <xsl:element name="table">
            <xsl:attribute name="width">100%</xsl:attribute>
            <xsl:attribute name="border">0</xsl:attribute>
            <xsl:attribute name="cellspacing">0</xsl:attribute>
            <xsl:attribute name="cellpadding">4</xsl:attribute>
            <xsl:element name="tr">
              <xsl:element name="td">
                <xsl:attribute name="class">howtogetheader</xsl:attribute>
                <xsl:value-of select="."/>
                <xsl:apply-templates/>
              </xsl:element>
            </xsl:element>
            <xsl:element name="tr">
              <xsl:element name="td">
                <xsl:attribute name="class">howtogetbody</xsl:attribute>
                <xsl:apply-templates select="$EmbeddedDoc//help:to-popup[@Eid=$Docpart]" mode="popup"/>
              </xsl:element>
            </xsl:element>
          </xsl:element>
        </xsl:if>
        <xsl:element name="br"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="Link" select="concat($hp,$sm,$Database,$sl,@Id,$qm,'Language',$es,$Language,$am,'System',$es,$System,$am,'Eid',$es,@Eid)"/>
        <xsl:element name="a">
          <xsl:attribute name="class">ContentLink</xsl:attribute>
          <xsl:attribute name="href"><xsl:value-of select="$Link"/></xsl:attribute>
          <xsl:attribute name="name">	<xsl:call-template name="linkname"/></xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF ANCHORS                                            |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | These can be based on help index keywords or Help-IDs            |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Process anchor names (help index keywords) -->
  <xsl:template match="help:key-word">
    <xsl:element name="a">
      <xsl:attribute name="name"><xsl:value-of select="@tag"/></xsl:attribute>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:key-word" mode="doctitle">
    <xsl:element name="a">
      <xsl:attribute name="name"><xsl:value-of select="@tag"/></xsl:attribute>
    </xsl:element>
  </xsl:template>
  <!-- Process anchor names (help ids) -->
  <xsl:template match="help:help-id">
    <xsl:element name="a">
      <xsl:attribute name="name"><xsl:value-of select="@value"/></xsl:attribute>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:help-id" mode="doctitle">
    <xsl:element name="a">
      <xsl:attribute name="name"><xsl:value-of select="@value"/></xsl:attribute>
    </xsl:element>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF LINKS                                              |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Process help links -->
  <xsl:template match="help:link">
    <xsl:choose>
      <xsl:when test="@Eid">
        <xsl:variable name="Link" select="concat($hp,$sm,$Database,$sl,@Id,$qm,'Language',$es,$Language,$am,'System',$es,$System,'#',@Eid)"/>
        <xsl:element name="a">
          <xsl:attribute name="class">ContentLink</xsl:attribute>
          <xsl:attribute name="href"><xsl:value-of select="$Link"/></xsl:attribute>
          <xsl:attribute name="name">	<xsl:call-template name="linkname"/></xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="Link" select="concat($hp,$sm,$Database,$sl,@Id,$qm,'Language',$es,$Language,$am,'System',$es,$System)"/>
        <xsl:element name="a">
          <xsl:attribute name="class">ContentLink</xsl:attribute>
          <xsl:attribute name="href"><xsl:value-of select="$Link"/></xsl:attribute>
          <xsl:attribute name="name">	<xsl:call-template name="linkname"/></xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="help:link" mode="embedded">
    <xsl:choose>
      <xsl:when test="@Eid">
        <xsl:variable name="Link" select="concat($hp,$sm,$Database,$sl,@Id,$qm,'Language',$es,$Language,$am,'System',$es,$System,'#',@Eid)"/>
        <xsl:element name="a">
          <xsl:attribute name="class">ContentLink</xsl:attribute>
          <xsl:attribute name="href"><xsl:value-of select="$Link"/></xsl:attribute>
          <xsl:attribute name="name">	<xsl:call-template name="linkname"/></xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="Link" select="concat($hp,$sm,$Database,$sl,@Id,$qm,'Language',$es,$Language,$am,'System',$es,$System)"/>
        <xsl:element name="a">
          <xsl:attribute name="class">ContentLink</xsl:attribute>
          <xsl:attribute name="href"><xsl:value-of select="$Link"/></xsl:attribute>
          <xsl:attribute name="name">	<xsl:call-template name="linkname"/></xsl:attribute>
          <xsl:apply-templates/>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="help:link" mode="doctitle">
    <xsl:apply-templates/>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF IMAGES                                              |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Process images -->
  <xsl:template match="draw:image | office:image">
    <xsl:element name="img">
      <xsl:variable name="pictureLink" select="concat($hp,$sm,'picture',$sl,@xlink:href,$qm,'Language',$es,$Language,$am,'System',$es,$System)"/>
      <xsl:attribute name="src"><xsl:value-of select="$pictureLink"/></xsl:attribute>
		<xsl:attribute name="width"><xsl:value-of select="@svg:pixelx"/></xsl:attribute>
	   <xsl:attribute name="height"><xsl:value-of select="@svg:pixely"/></xsl:attribute>
	   <xsl:attribute name="alt"><xsl:value-of select="@svg:desc"/></xsl:attribute>

<!--
		Width/Height is now contained in the svg:pixelx and svg:pixely attributes
      <xsl:attribute name="width"><xsl:value-of select="round(substring-before(@svg:width,'cm')*37.8)"/></xsl:attribute>
      <xsl:attribute name="height"><xsl:value-of select="round(substring-before(@svg:height,'cm')*37.8)"/></xsl:attribute>
//-->
    </xsl:element>
  </xsl:template>
  <!-- Remove residues of formatting information -->
  <xsl:template match="style"/>
  <!-- Add CSS link to custom.css -->
  <xsl:template match="help:css-file-link">
    <xsl:variable name="CssStylesheet" select="concat($hp,$sm,$cs,$sl,$qm,'Language',$es,$Language,$am,'System',$es,$System)"/>
    <xsl:element name="link">
      <xsl:attribute name="rel">STYLESHEET</xsl:attribute>
      <xsl:attribute name="href"><xsl:value-of select="$CssStylesheet"/></xsl:attribute>
      <xsl:attribute name="type">text/css</xsl:attribute>
    </xsl:element>
    <!-- Add Encoding meta tag for correct display of asian text -->
    <xsl:element name="meta">
      <xsl:attribute name="http-equiv">Content-type</xsl:attribute>
      <xsl:attribute name="content">text/html; charset=utf-8</xsl:attribute>
    </xsl:element>
  </xsl:template>
  
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | ERROR HANDLING                                                   |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- error handling -->
  <xsl:template match="help:error-id">
    <xsl:value-of select="$Id"/>
  </xsl:template>
  <xsl:template match="help:help-id-missing">
    <xsl:value-of select="$Id"/>
  </xsl:template>
  <xsl:template match="help:error-module">
    <xsl:value-of select="$Database"/>
  </xsl:template>
  <xsl:template match="help:help-mod-missing">
    <xsl:value-of select="$Database"/>
  </xsl:template>
  <xsl:template match="help:help-org-missing">
    <xsl:value-of select="$Path"/>
  </xsl:template>
  <xsl:template match="help:error-path">
    <xsl:value-of select="$Path"/>
  </xsl:template>
 
  <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | PROCESSING OF PRODUCT INFORMATION/BRANDING                       |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
  
  <!-- Branding -->
  <xsl:template match="help:productname">
    <xsl:element name="span">
      <xsl:attribute name="class">prod</xsl:attribute>
      <xsl:value-of select="$productname"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:productversion">
    <xsl:element name="span">
      <xsl:attribute name="class">prod</xsl:attribute>
      <xsl:value-of select="$productversion"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:productpath">{installpath}</xsl:template>
  <xsl:template match="help:vendorname">
    <xsl:element name="span">
      <xsl:attribute name="class">vendor</xsl:attribute>
      <xsl:value-of select="$vendorname"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:vendorversion">
    <xsl:element name="span">
      <xsl:attribute name="class">vendor</xsl:attribute>
      <xsl:value-of select="$vendorversion"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:vendorpath">{installpath}</xsl:template>
  <xsl:template match="help:vendorshort">
    <xsl:element name="span">
      <xsl:attribute name="class">vendor</xsl:attribute>
      <xsl:value-of select="$vendorshort"/>
    </xsl:element>
  </xsl:template>
  <xsl:template match="help:productname" mode="doctitle">
    <xsl:value-of select="$productname"/>
  </xsl:template>
  <xsl:template match="help:productversion" mode="doctitle">
    <xsl:value-of select="$productversion"/>
  </xsl:template>
  <xsl:template match="help:productpath" mode="doctitle">{installpath}</xsl:template>
  <xsl:template match="help:vendorname" mode="doctitle">
    <xsl:value-of select="$vendorname"/>
  </xsl:template>
  <xsl:template match="help:vendorversion" mode="doctitle">
    <xsl:value-of select="$vendorversion"/>
  </xsl:template>
  <xsl:template match="help:vendorpath" mode="doctitle">{installpath}</xsl:template>
  <xsl:template match="help:vendorshort" mode="doctitle">
    <xsl:value-of select="$vendorshort"/>
  </xsl:template>
  <xsl:template match="help:productname" mode="popup">
    <xsl:value-of select="$productname"/>
  </xsl:template>
  <xsl:template match="help:productversion" mode="popup">
    <xsl:value-of select="$productversion"/>
  </xsl:template>
  <xsl:template match="help:productpath" mode="popup">{installpath}</xsl:template>
  <xsl:template match="help:vendorname" mode="popup">
    <xsl:value-of select="$vendorname"/>
  </xsl:template>
  <xsl:template match="help:vendorversion" mode="popup">
    <xsl:value-of select="$vendorversion"/>
  </xsl:template>
  <xsl:template match="help:vendorpath" mode="popup">{installpath}</xsl:template>
  <xsl:template match="help:vendorshort" mode="popup">
    <xsl:value-of select="$vendorshort"/>
  </xsl:template>
  <xsl:template match="help:productname" mode="embedded">
    <xsl:value-of select="$productname"/>
  </xsl:template>
  <xsl:template match="help:productversion" mode="embedded">
    <xsl:value-of select="$productversion"/>
  </xsl:template>
  <xsl:template match="help:productpath" mode="embedded">{installpath}</xsl:template>
  <xsl:template match="help:vendorname" mode="embedded">
    <xsl:value-of select="$vendorname"/>
  </xsl:template>
  <xsl:template match="help:vendorversion" mode="embedded">
    <xsl:value-of select="$vendorversion"/>
  </xsl:template>
  <xsl:template match="help:vendorpath" mode="embedded">{installpath}</xsl:template>
  <xsl:template match="help:vendorshort" mode="embedded">
    <xsl:value-of select="$vendorshort"/>
  </xsl:template>
  
   <!-- 
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  | ADDITIONAL STUFF                                                 |
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  -->
   
  <!-- admin info -->
  <xsl:template match="referenznumber"/>
  <!-- spacing -->
  <xsl:template match="text:s">
    <!-- not currentyl working -->
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="text:tab-stop">
    <xsl:call-template name="spacer">
      <xsl:with-param name="width">
        <xsl:value-of select="$tabwidth"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:template>
   
  <xsl:template name="loop">
    <xsl:param name="ct">1</xsl:param>
    <xsl:param name="cur">1</xsl:param>
    <xsl:param name="wdth">1</xsl:param>
    <xsl:call-template name="spacer">
      <xsl:with-param name="width">
        <xsl:value-of select="$wdth"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:if test="$cur &lt; $ct">
      <xsl:call-template name="loop">
        <xsl:with-param name="ct">
          <xsl:value-of select="$ct"/>
        </xsl:with-param>
        <xsl:with-param name="cur">
          <xsl:value-of select="$cur+1"/>
        </xsl:with-param>
        <xsl:with-param name="wdth">
          <xsl:value-of select="$wdth"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
  
  <xsl:template name="spacer">
    <xsl:param name="width">1</xsl:param>
    <xsl:element name="img">
      <xsl:variable name="pictureLink" select="concat($hp,$sm,'picture',$sl,$onepixel,$qm,'Language',$es,$Language,$am,'System',$es,$System)"/>
      <xsl:attribute name="src"><xsl:value-of select="$pictureLink"/></xsl:attribute>
      <xsl:attribute name="width"><xsl:value-of select="$width"/></xsl:attribute>
      <xsl:attribute name="height">1</xsl:attribute>
    </xsl:element>
  </xsl:template>
  
  <xsl:template name="linkname">
  	<xsl:call-template name="string-replace">
  		<xsl:with-param name="string"><xsl:value-of select="."/></xsl:with-param>
  		<xsl:with-param name="from">%PRODUCTNAME</xsl:with-param>
  		<xsl:with-param name="to"><xsl:value-of select="$productname"/></xsl:with-param>
  	</xsl:call-template>
  </xsl:template>

	<xsl:template name="string-replace" >
     <xsl:param name="string"/>
     <xsl:param name="from"/>
     <xsl:param name="to"/>
     <xsl:choose>
       <xsl:when test="contains($string,$from)">
         <xsl:value-of select="substring-before($string,$from)"/>
         <xsl:value-of select="$to"/>
         <xsl:call-template name="string-replace">
         <xsl:with-param name="string" 
             select="substring-after($string,$from)"/>
         <xsl:with-param name="from" select="$from"/>
         <xsl:with-param name="to" select="$to"/>
         </xsl:call-template>
       </xsl:when>
       <xsl:otherwise>
         <xsl:value-of select="$string"/>
       </xsl:otherwise>
     </xsl:choose>
   </xsl:template>

  <!--
    Fix bug 1120 by replacing the OOo support URL
  -->
  <xsl:template match="a">
    <xsl:choose>
      <xsl:when test="@href='http://www.openoffice.org/welcome/support.html'">
        <a href="http://trinity.neooffice.org/modules.php?name=Forums">http://trinity.neooffice.org/modules.php?name=Forums</a>
      </xsl:when>
      <xsl:otherwise>
        <xsl:copy>
          <xsl:apply-templates select="*|@*|comment()|processing-instruction()|text()"/>
        </xsl:copy>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
