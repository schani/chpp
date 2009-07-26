Summary: chpp is the chakotay preprocessor language
Name: chpp
Version: 0.3.6
Release: 2
License: GNU General Public License
Group: Utilities/Text
Source: %{name}_%{version}-1.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-build
%description
chpp is a preprocessor which can be applied to a myriad of applications.

%prep
%setup

%build
./configure --prefix=%{_prefix} --datarootdir=%{_datadir} --bindir=%{_bindir}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
rm -f $RPM_BUILD_ROOT/%{_infodir}/dir

%files
%defattr(-,root,root)
%doc AUTHORS TODO NEWS INSTALL README COPYING ChangeLog

%{_infodir}/chpp.info*
%{_datadir}/chpp/include
%{_bindir}/chpp

%post
%info_add %{name}.info

%pre
%info_del %{name}.info
