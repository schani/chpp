Summary: chpp is the chakotay preprocessor language
Name: chpp
Version: 0.3.6
Release: 1
Copyright: GPL
Group: Utilities/Text
Source: chpp-0.3.6.tar.gz
%description
chpp is a preprocessor which can be applied to a myriad of applications.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
make install

%files
%doc AUTHORS TODO NEWS INSTALL README COPYING ChangeLog

/usr/info/chpp.info*
/usr/lib/chpp/include
/usr/bin/chpp
