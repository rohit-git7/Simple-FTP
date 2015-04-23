

int sys_auth_user(const char *username, const char *password)
{
  struct passwd *pw;
  struct spwd *sp;
  char *encrypted, *correct;

  pw = getpwnam(username);
  endpwent();

  if (!pw) 
	return 1; //user doesn't really exist

  sp = getspnam (pw->pw_name);
  endspent();
  if(sp)
     correct = sp->sp_pwdp;
  else
     correct = pw->pw_passwd;

  encrypted = crypt(password, correct);
  return strcmp (encrypted, correct) ? 2 : 0;  // bad password=2, success=0
}

