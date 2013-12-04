To build the Windows Phone projects, you need to install the Windows Live SDK first (SkyDrive API):
http://msdn.microsoft.com/en-us/library/live/hh826550.aspx

To make the SkyDrive API work, you need to add your own Client ID in the file MainPage.xaml in the following part (replace <CLIENT ID>):

<Controls:SignInButton Content="SignInButton" Name="signinButton"
                                ClientId="<CLIENT ID>"
                                Branding="Skydrive"
                                Scopes="wl.signin wl.skydrive_update wl.offline_access"
                                SessionChanged="btnSignin_SessionChanged" Width="200" Height="72" VerticalAlignment="Top"/>

You can acquire your own client id at https://account.live.com/developers/applications/create