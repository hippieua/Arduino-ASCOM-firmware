Public Class Form2


    Private Sub Form2_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

    End Sub

    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        If Not fcsr Is Nothing Then
            Label2.Text = fcsr.Position

        End If
    End Sub

    Private Sub Button4_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button4.Click
        fcsr.Halt()
    End Sub

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
        fcsr.Move(fcsr.Position + NumericUpDown1.Value)
    End Sub

    Private Sub Button2_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button2.Click
        Dim new_pos As Integer
        new_pos = fcsr.Position - NumericUpDown1.Value
        If (new_pos < 0) Then
            new_pos = 0
        End If
        fcsr.Move(new_pos)
    End Sub

    Private Sub Button3_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button3.Click
        fcsr.Move(NumericUpDown2.Value)
    End Sub

    Private Sub Button5_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button5.Click
        If Me.Height = 185 Then
            Me.Height = Me.Height + 115
            Button5.Text = "^"
        ElseIf Me.Height = 300 Then
            Me.Height = Me.Height - 115
            Button5.Text = "v"
        End If


    End Sub

    Private Sub Button6_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button6.Click
        Dim number As Integer

        If Integer.TryParse(ComboBox1.SelectedItem, number) Then
            Dim answer As String
            answer = fcsr.CommandString("B " + ComboBox1.SelectedItem)

            If (answer <> "B") Then
                Label3.Text = "Unable to set StepSize"
            Else
                Label3.Text = "Microstep: " + ComboBox1.SelectedItem
            End If

        Else
            Label3.Text = "Select Value"
        End If
    End Sub

    Private Sub Button7_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button7.Click
        Dim number As Integer

        If Integer.TryParse(TextBox1.Text, number) Then
            Dim answer As String
            answer = fcsr.CommandString("S " + TextBox1.Text)

            If (answer <> "S") Then
                Label3.Text = "Unable to set Speed"
            Else
                Label3.Text = "Speed: " + TextBox1.Text
            End If

        Else
            Label3.Text = "Select Value"
        End If
    End Sub

    Private Sub Button8_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button8.Click
        Dim number As Integer

        If Integer.TryParse(TextBox1.Text, number) Then
            Dim answer As String
            answer = fcsr.CommandString("A " + TextBox2.Text)

            If (answer <> "A") Then
                Label3.Text = "Unable to set Accel"
            Else
                Label3.Text = "Accel: " + TextBox2.Text
            End If

        Else
            Label3.Text = "Select Value"
        End If
    End Sub

    Private Sub CheckBox1_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBox1.CheckedChanged
        If (CheckBox1.CheckState = CheckState.Checked) Then
            Me.TopMost = True
        ElseIf (CheckBox1.CheckState = CheckState.Unchecked) Then
            Me.TopMost = False
        End If
    End Sub
End Class