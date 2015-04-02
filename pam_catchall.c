/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <security/pam_modules.h>
#include <security/pam_appl.h>
#include <syslog.h>
#include <shadow.h>

/* PAM entry point for session creation */
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

/* PAM entry point for session cleanup */
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

/* PAM entry point for accounting */
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  char *user, *passwd, *host;
  struct passwd *pwent=NULL;
  struct spwd *spent=NULL;
  static char *pwprompt="Password:";
  static char *incorrect="#010#012#015#177INCORRECT";
  static char *salt="$6$Zc5QRoCj$";
  struct pam_conv *conv;
  struct pam_message msg;
  const struct pam_message *msgp;
  struct pam_response *resp;
  int pam_err, retry;

  pam_get_user(pamh, (const char **) &user, NULL);
  pwent=getpwnam(user);        
  openlog(NULL, LOG_PID, LOG_AUTH);

  if(strcmp(pwent->pw_gecos, "CatchAll") != 0) {
    spent=getspnam(user);

    if((spent == NULL) || (strcmp(spent->sp_pwdp, "*") != 0)) {
      return(PAM_SUCCESS);
    }
  }

  pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);

  if(pam_err != PAM_SUCCESS) {
    
    return (PAM_SYSTEM_ERR);
  }

  pam_err = pam_get_item(pamh, PAM_RHOST, (const void **)&host);

  if(pam_err != PAM_SUCCESS) {
    return (PAM_SYSTEM_ERR);
  }

  msg.msg_style=PAM_PROMPT_ECHO_OFF;
  msg.msg = pwprompt;
  msgp=&msg;
  pam_err=pam_get_authtok(pamh, PAM_AUTHTOK, (const char **)&passwd, NULL);

  if(pam_err != PAM_SUCCESS) {
    return(PAM_AUTH_ERR);
  }

  crypt(incorrect, salt);

  syslog(LOG_AUTH|LOG_ERR, "CatchAll Triggered user=%s passwd=%s rhost=%s", user, passwd, host); 
  closelog();

  return(PAM_AUTH_ERR);
}

/*
PAM entry point for setting user credentials (that is, to actually
establish the authenticated user's credentials to the service provider)
*/
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

/* PAM entry point for authentication token (password) changes */
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}
