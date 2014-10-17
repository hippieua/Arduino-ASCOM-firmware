Public Class Form1

    Private driver As ASCOM.DriverAccess.Focuser

    'This event is where the driver is choosen. The device ID will be saved in the settings.
    Private Sub buttonChoose_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonChoose.Click
        My.Settings.DriverId = ASCOM.DriverAccess.Focuser.Choose(My.Settings.DriverId)
        SetUIState()
    End Sub

    ''' <summary>
    ''' Connects to the device to be tested.
    ''' </summary>
    ''' <param name="sender">The source of the event.</param>
    ''' <param name="e">The <see cref="System.EventArgs" /> instance containing the event data.</param>
    Private Sub buttonConnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonConnect.Click
        If (IsConnected) Then
            driver.Connected = False





        Else
            Try
                driver = New ASCOM.DriverAccess.Focuser(My.Settings.DriverId)

                driver.Connected = True



                If (driver.Temperature = 0) Then
                    Label6.Text = "Not supported"
                Else
                    Label6.Text = driver.Temperature
                End If

                If (driver.Absolute = True) Then
                    Label7.Text = "Absolute"
                Else
                    Label7.Text = "Relative"
                End If



            Catch ex As Exception
                MsgBox("Error: " + ex.Message, MsgBoxStyle.OkOnly, "Error")
            End Try

        End If
        SetUIState()
    End Sub

    Private Sub Form1_FormClosing(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles MyBase.FormClosing
        If IsConnected Then
            driver.Connected = False
        End If
        ' the settings are saved automatically when this application is closed.
    End Sub

    ''' <summary>
    ''' Sets the state of the UI depending on the device state
    ''' </summary>
    Private Sub SetUIState()
        buttonConnect.Enabled = Not String.IsNullOrEmpty(My.Settings.DriverId)
        buttonChoose.Enabled = Not IsConnected
        buttonSetup.Enabled = Not IsConnected
        buttonMoveIn.Enabled = IsConnected
        buttonMoveOut.Enabled = IsConnected
        buttonMoveTo.Enabled = IsConnected
        buttonConnect.Text = IIf(IsConnected, "Disconnect", "Connect")

    End Sub

    ''' <summary>
    ''' Gets a value indicating whether this instance is connected.
    ''' </summary>
    ''' <value>
    ''' 
    ''' <c>true</c> if this instance is connected; otherwise, <c>false</c>.
    ''' 
    ''' </value>
    Private ReadOnly Property IsConnected() As Boolean
        Get
            If Me.driver Is Nothing Then Return False
            Return driver.Connected
        End Get
    End Property

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonMoveTo.Click
        driver.Move(NUD_Absolute.Value)
    End Sub

    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        If IsConnected Then
            Label5.Text = driver.Position
        End If
    End Sub

    Private Sub Button2_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonSetup.Click
        driver = New ASCOM.DriverAccess.Focuser(My.Settings.DriverId)
        driver.SetupDialog()
    End Sub

    Private Sub Button3_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonMoveOut.Click
        driver.Move(driver.Position + NUD_Relative.Value)
    End Sub

    Private Sub Button2_Click_1(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles buttonMoveIn.Click
        Dim new_pos As Integer
        new_pos = driver.Position - NUD_Relative.Value
        If (new_pos < 0) Then
            new_pos = 0
        End If
        driver.Move(new_pos)
    End Sub
End Class
