Attribute VB_Name = "VbaCallCPG"
#If VBA7 Then
  Private Declare PtrSafe Function vbadll Lib "lycpg64.cpg" (ByVal code As Long, ByVal x As Double) As Long
#Else
  Private Declare Function vbadll Lib "lycpg32.cpg" (ByVal code As Long, ByVal x As Double) As Long
#End If

Sub VBA调用CPG_CDR复制物件到AI()
 ret = vbadll(2, 0)
 MsgBox "CDR复制物件到AI 功能 " & ret
End Sub

Sub AI复制物件到CDR()
 ret = vbadll(1, 0)
 MsgBox "AI复制物件到CDR 功能 " & ret
End Sub

Sub Clear_Fill()
 ret = vbadll(3, 0)
 MsgBox "Clear_Fill功能 " & ret
End Sub


Sub cql_FillColor()
 ret = vbadll(5, 0)
End Sub


Sub Shapes_Filp()
 ret = vbadll(4, 0)
End Sub

Sub cql_OutlineColor()
 ret = vbadll(6, 0)
End Sub

Sub cql_SameSize()
 ret = vbadll(7, 0)
End Sub

Sub fill_red()
 ret = vbadll(8, 0)
End Sub

'// 第9号功能: 按边界批量绘制矩形  参数:正数向外，负数向内
Sub BBox_DrawRectangle()
 ret = vbadll(9, 0#)
End Sub

'// 第10号功能: 智能群组功能  参数:正数 向外容差  负数 向内容差
Sub Box_AutoGroup()
 ret = vbadll(10, 1#)
End Sub

'// 第11号功能: 异形群组功能
Sub BoundaryGroup()
 ret = vbadll(11, 0)
End Sub

Sub Open_LYCPG()
 ret = vbadll(888, 8)
End Sub