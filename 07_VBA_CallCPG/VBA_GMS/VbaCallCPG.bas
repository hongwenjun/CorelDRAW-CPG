Attribute VB_Name = "VbaCallCPG"
Private Declare PtrSafe Function vbadll Lib "C:\Program Files\Corel\CorelDRAW Graphics Suite 2020\Draw\Plugins64\lycpg64.cpg" (ByVal code As Long) As Long

Sub VBA调用CPG_CDR复制物件到AI()
 ret = vbadll(2)
 MsgBox "CDR复制物件到AI 功能 " & ret
End Sub

Sub AI复制物件到CDR()
 ret = vbadll(1)
 MsgBox "AI复制物件到CDR 功能 " & ret
End Sub

Sub Clear_Fill()
 ret = vbadll(3)
 MsgBox "Clear_Fill功能 " & ret
End Sub


Sub cql_FillColor()
 ret = vbadll(5)
End Sub


Sub Shapes_Filp()
 ret = vbadll(4)
End Sub

Sub cql_OutlineColor()
 ret = vbadll(6)
End Sub

Sub cql_SameSize()
 ret = vbadll(7)
End Sub

Sub fill_red()
 ret = vbadll(8)
End Sub
