#include <stdio.h>
#include <shadow.h>

int main()
{
    struct spwd* shadow_passwd;
    char username[128] = {0};
    printf("Enter your username: ");
    scanf("%s", username);
    if((shadow_passwd = getspnam(username)) != NULL)
        printf("%s's encrypted password is %s\n", username, shadow_passwd->sp_pwdp);

    return 0;
}
