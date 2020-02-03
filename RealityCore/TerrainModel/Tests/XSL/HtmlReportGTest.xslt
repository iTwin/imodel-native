<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:msxsl="urn:schemas-microsoft-com:xslt">
	<xsl:output method="html" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"/>
	
	<xsl:template match="/testsuites">
		<html>
			<head>
				<title>ATP Execution Report</title>
				<style>
					body 						{ font: small verdana, arial, helvetica; color:#000000;	}
					.coverageReportTable		{ font-size: 9px; }
					.reportHeader 				{ padding: 5px 8px 5px 8px; font-size: 12px; border: 1px solid; margin: 0px;	}
					.titleText					{ font-weight: bold; font-size: 12px; white-space: nowrap; padding: 0px; margin: 1px; }
					.subtitleText 				{ font-size: 9px; font-weight: normal; padding: 0px; margin: 1px; white-space: nowrap; }
					.projectStatistics			{ font-size: 10px; border-left: #649cc0 1px solid; white-space: nowrap;	width: 40%;	}
					.heading					{ font-weight: bold; }
					.mainTableHeaderLeft 		{ border: #dcdcdc 1px solid; font-weight: bold;	padding-left: 5px; }
					.mainTableHeader 			{ border-bottom: 1px solid; border-top: 1px solid; border-right: 1px solid;	text-align: center;	}
					.mainTableGraphHeader 		{ border-bottom: 1px solid; border-top: 1px solid; border-right: 1px solid;	text-align: left; font-weight: bold; }
					.mainTableCellItem 			{ background: #ffffff; border-left: #dcdcdc 1px solid; border-right: #dcdcdc 1px solid; padding-left: 10px; padding-right: 10px; font-weight: bold; font-size: 10px; }
					.mainTableCellData 			{ background: #ffffff; border-right: #dcdcdc 1px solid;	text-align: center;	white-space: nowrap; }
					.mainTableCellPercent 		{ background: #ffffff; font-weight: bold; white-space: nowrap; text-align: right; padding-left: 10px; }
					.mainTableCellGraph 		{ background: #ffffff; border-right: #dcdcdc 1px solid; padding-right: 5px; }
					.mainTableCellBottom		{ border-bottom: #dcdcdc 1px solid;	}
					.childTableHeader 			{ border-top: 1px solid; border-bottom: 1px solid; border-left: 1px solid; border-right: 1px solid;	font-weight: bold; padding-left: 10px; }
					.childTableCellIndentedItem { background: #ffffff; border-left: #dcdcdc 1px solid; border-right: #dcdcdc 1px solid; padding-right: 10px; font-size: 10px; }		
					.exclusionTableCellItem 	{ background: #ffffff; border-left: #dcdcdc 1px solid; border-right: #dcdcdc 1px solid; padding-left: 10px; padding-right: 10px; }
					.projectTable				{ background: #a9d9f7; border-color: #649cc0; }
					.primaryTable				{ background: #d7eefd; border-color: #a4dafc; }
					.secondaryTable 			{ background: #f9e9b7; border-color: #f6d376; }
					.secondaryChildTable 		{ background: #fff6df; border-color: #f5e1b1; }
					.exclusionTable				{ background: #fadada; border-color: #f37f7f; }
					.graphBarNotVisited			{ font-size: 2px; border:#9c9c9c 1px solid; background:#df0000; }
					.graphBarSatisfactory		{ font-size: 2px; border:#9c9c9c 1px solid;	background:#f4f24e; }
					.graphBarVisited			{ background: #00df00; font-size: 2px; border-left:#9c9c9c 1px solid; border-top:#9c9c9c 1px solid; border-bottom:#9c9c9c 1px solid; }
					.graphBarVisitedFully		{ background: #00df00; font-size: 2px; border:#9c9c9c 1px solid; }
				</style>
			</head>
			<body>
				<table class="coverageReportTable" cellpadding="2" cellspacing="0">
					<tbody>

  		<xsl:call-template name="header" />
         
		<xsl:call-template name="suiteSummary"/>

	    	<xsl:call-template name="atpSummary" /> 
    
		<xsl:call-template name="footer" />  

					</tbody>
				</table>
			</body>
		</html>



	</xsl:template>	
	
	<!-- Report Header -->
	<xsl:template name="header">
				<tr>
					<td class="projectTable reportHeader" colspan="7">
						<table width="100%">
							<tbody>
								<tr>
									<td valign="top">
										<h1 class="titleText">ATP Execution Report</h1>
										<table cellpadding="1" class="subtitleText">
											<tbody>
												<tr>
													<td class="heading">Report generated on:</td>
													<td>Can Not Decide</td>
												</tr>
												<tr>
													<td class="heading">NUnit version:</td>
													<td>GTEST</td>
												</tr>
												<tr>
													<td class="heading">CLR Version:</td>
													<td></td>
												</tr>
											</tbody>
										</table>
									</td>
                                    <!--td class="projectStatistics" align="left" valign="top">
										<table cellpadding="1">
											<tbody>
												<tr>
													<td rowspan="5" valign="top" colspan="5" align="left" nowrap="true" class="heading">ATP Statistics:</td>
                     										</tr>
												<tr>
													<td>&#160;&#160;</td>
													<td align="right" class="heading">Tests run:</td>
													<td>&#160;&#160;</td>
													<td align="right"><xsl:value-of select="@tests" /></td>
												</tr>
												<tr>
													<td>&#160;&#160;</td>
													<td align="right" class="heading">Failures:</td>
													<td>&#160;&#160;</td>
													<td align="right"><xsl:value-of select="@failures" /></td>
												</tr>
												<tr>
													<td>&#160;&#160;</td>
													<td align="right" class="heading">Not run:</td>
													<td>&#160;&#160;</td>
													<td align="right"><xsl:value-of select="@disabled" /></td>
												</tr>
												<tr>
													<td>&#160;&#160;</td>
													<td align="right" class="heading">Time:</td>
													<td>&#160;&#160;</td>
													<td align="right"><xsl:value-of select="sum(./test-results/test-suite/@time)" /> sec</td>
												</tr>
											</tbody>
										</table>
									</td-->
								</tr>
							</tbody>
						</table>
					</td>
				</tr>
	</xsl:template>
	
	<!-- Suite Summary -->
	<xsl:template name="suiteSummary">
				<tr>
					<td colspan="7">&#160;</td>
				</tr>
				<tr>
					<td class="primaryTable mainTableHeaderLeft">Suite</td>
					<td class="primaryTable mainTableGraphHeader" colspan="2">Success Graph</td>
					<td class="primaryTable mainTableHeader">Total</td>
					<td class="primaryTable mainTableHeader">Failures</td>
					<td class="primaryTable mainTableHeader">Not Run</td>
					<td class="primaryTable mainTableHeader">Time</td>
				</tr>
        <!--xsl:for-each select="//test-results"-->
			<xsl:call-template name="coverageDetail">
				<xsl:with-param name="name" select="./@name" />
				<xsl:with-param name="total" select="./@tests" />
				<xsl:with-param name="failures" select="./@failures" />
				<xsl:with-param name="not-run" select="./@disabled" />
				<xsl:with-param name="time" select="./@time" />
			</xsl:call-template>
        <!--/xsl:for-each-->
	</xsl:template>
		
		
	<!-- ATP Summary -->
	<xsl:template name="atpSummary">
		<xsl:for-each select="//testsuite">
				<tr>
					<td colspan="7">&#160;</td>
				</tr>
				<tr>
					<td class="secondaryTable mainTableHeaderLeft">Suite</td>
					<td class="secondaryTable mainTableGraphHeader" colspan="2">Success Graph</td>
					<td class="secondaryTable mainTableHeader">Total</td>
					<td class="secondaryTable mainTableHeader">Failures</td>
					<td class="secondaryTable mainTableHeader">Not Run</td>
					<td class="secondaryTable mainTableHeader">Time</td>
				</tr>				
			<xsl:call-template name="coverageDetailSecondary">
				<xsl:with-param name="name" select="./@name" />
				<xsl:with-param name="total" select="./@tests" />
				<xsl:with-param name="failures" select="./@failures" />
				<xsl:with-param name="not-run" select="./@disabled" />
				<xsl:with-param name="time" select="./@time" />
			</xsl:call-template>
				<tr>
					<td class="secondaryChildTable childTableHeader" colspan="4">Test</td>
					<td class="secondaryChildTable childTableHeader" >Executed</td>
					<td class="secondaryChildTable childTableHeader" >Success</td>
					<td class="secondaryChildTable childTableHeader" >Time</td>
				</tr>				
			<xsl:for-each select=".//testcase">
				<xsl:call-template name="coverageIndentedDetail">
					<xsl:with-param name="name" select="./@name" />
					<xsl:with-param name="executed" select="./@status" />
					<xsl:with-param name="success" select="./@status" />
					<xsl:with-param name="time" select="./@time" />
					<xsl:with-param name="styleTweak">padding-left:20px</xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
		</xsl:for-each>
	</xsl:template>	
	
	<!-- Coverage detail row in main grid displaying a name, statistics and graph bar -->
	<xsl:template name="coverageDetail">
		<xsl:param name="name" />
		<xsl:param name="total" />
		<xsl:param name="failures" />
		<xsl:param name="not-run" />
		<xsl:param name="time" />
				<tr>
					<td class="mainTableCellBottom mainTableCellItem">
                                             <xsl:call-template name="lastIndexOf">
                                                <xsl:with-param name="string" select="$name" />
                                                <xsl:with-param name="char">\</xsl:with-param>
                                             </xsl:call-template>
                                        </td>
					<td class="mainTableCellBottom mainTableCellGraph" colspan="2">
						<xsl:call-template name="detailPercent">
							<xsl:with-param name="notVisited" select="$not-run + $failures" />
							<xsl:with-param name="total" select="$total" />
							<xsl:with-param name="scale" select="200" />
						</xsl:call-template>
					</td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$total" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$failures" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$not-run" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$time" /></td>
				</tr>
	</xsl:template>
	
	<!-- Coverage detail row in secondary grid header displaying a name, statistics and graph bar -->
	<xsl:template name="coverageDetailSecondary">
		<xsl:param name="name" />
		<xsl:param name="total" />
		<xsl:param name="failures" />
		<xsl:param name="not-run" />
		<xsl:param name="time" />
				<tr>
					<td class="mainTableCellItem">
                                             <xsl:call-template name="lastIndexOf">
                                                <xsl:with-param name="string" select="$name" />
                                                <xsl:with-param name="char">\</xsl:with-param>
                                             </xsl:call-template>
                                        </td>
					<td class="mainTableCellGraph" colspan="2">
						<xsl:call-template name="detailPercent">
							<xsl:with-param name="notVisited" select="$not-run + $failures" />
							<xsl:with-param name="total" select="$total" />
							<xsl:with-param name="scale" select="200" />
						</xsl:call-template>
					</td>
					<td class="mainTableCellData"><xsl:value-of select="$total" /></td>
					<td class="mainTableCellData"><xsl:value-of select="$failures" /></td>
					<td class="mainTableCellData"><xsl:value-of select="$not-run" /></td>
					<td class="mainTableCellData"><xsl:value-of select="$time" /></td>
				</tr>
	</xsl:template>
	
	<!-- Coverage detail row with indented item name and shrunk graph bar -->
	<xsl:template name="coverageIndentedDetail">
		<xsl:param name="name" />
		<xsl:param name="executed" />
		<xsl:param name="success" />
		<xsl:param name="time" />
		<xsl:param name="styleTweak">padding-left:20px</xsl:param>
				<tr>
					<td class="mainTableCellBottom childTableCellIndentedItem" colspan="4"><xsl:attribute name="style"><xsl:value-of select="$styleTweak"/></xsl:attribute><xsl:value-of select="$name" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$executed" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$success" /></td>
					<td class="mainTableCellBottom mainTableCellData"><xsl:value-of select="$time" /></td>
				</tr>
	</xsl:template>		
	
	<!-- Footer -->
	<xsl:template name="footer">
				<tr>
					<td colspan="7">&#160;</td>
				</tr>
	</xsl:template>
	
	<!-- Draw % Green/Red/Yellow Bar -->
	<xsl:template name="detailPercent">
		<xsl:param name="notVisited" />
		<xsl:param name="total" />
		<xsl:param name="scale" />
		<xsl:variable name="visited" select="$total - $notVisited" />
		<xsl:variable name="coverage" select="$visited div $total * 100"/>
		<table cellpadding="0" cellspacing="0">
			<tbody>
				<tr>
					<xsl:if test="$notVisited = 0">
						<td class="graphBarVisitedFully" height="14">
							<xsl:attribute name="width">
								<xsl:value-of select="$scale" />
							</xsl:attribute>.</td>
					</xsl:if>
					<xsl:if test="($visited != 0) and ($notVisited != 0)">
						<td class="graphBarVisited" height="14">
							<xsl:attribute name="width">
								<xsl:value-of select="format-number($coverage div 100 * $scale, '0') - 1" />
							</xsl:attribute>.</td>
					</xsl:if>
					<xsl:if test="$notVisited != 0">
						<td class="graphBarSatisfactory" height="14">
							<xsl:attribute name="width">
								<xsl:value-of select="format-number($notVisited div $total * $scale, '0')" />
							</xsl:attribute>.</td>
					</xsl:if>
				</tr>
			</tbody>
		</table>
	</xsl:template>

 <xsl:template name="lastIndexOf">
    <!-- declare that it takes two parameters - the string and the char -->
    <xsl:param name="string" />
    <xsl:param name="char" />
    <xsl:choose>
       <!-- if the string contains the character... -->
       <xsl:when test="contains($string, $char)">
          <!-- call the template recursively... -->
          <xsl:call-template name="lastIndexOf">
             <!-- with the string being the string after the character
                  -->
             <xsl:with-param name="string"
                             select="substring-after($string, $char)" />
             <!-- and the character being the same as before -->
             <xsl:with-param name="char" select="$char" />
          </xsl:call-template>
       </xsl:when>
       <!-- otherwise, return the value of the string -->
       <xsl:otherwise><xsl:value-of select="$string" /></xsl:otherwise>
    </xsl:choose>
 </xsl:template>

</xsl:stylesheet>