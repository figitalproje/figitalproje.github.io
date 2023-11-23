function onSignIn(googleUser) {
    var profile = googleUser.getBasicProfile();
    console.log('ID: ' + profile.getId());
    console.log('Full Name: ' + profile.getName());
    console.log('Given Name: ' + profile.getGivenName());
    console.log('Family Name: ' + profile.getFamilyName());
    console.log('Image URL: ' + profile.getImageUrl());
    console.log('Email: ' + profile.getEmail());

    // Kullanıcı bilgilerini sayfada göster
    var userInfo = document.getElementById('user-info');
    userInfo.innerHTML = `
        <p>ID: ${profile.getId()}</p>
        <p>Full Name: ${profile.getName()}</p>
        <p>Email: ${profile.getEmail()}</p>
        <img src="${profile.getImageUrl()}" alt="Profile Image">
    `;
}

function signOut() {
    var auth2 = gapi.auth2.getAuthInstance();
    auth2.signOut().then(function () {
        console.log('User signed out.');
        document.getElementById('user-info').innerHTML = '';
    });
}
