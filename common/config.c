#include <stdio.h>
#include <string.h>
#include "config.h"
#include <stdlib.h>
#include <stdbool.h>
#include "tools.h"

void initConfig(Config *C){
    memset(C,0,sizeof(Config));
    strcpy(C->name,"common");
    strcpy(C->addr,"127.0.0.1");
    C->port = 7000;
    C->use_encryption = false;
    strcpy(C->encryption_key,"");
    strcpy(C->auth_token,"");
    strcpy(C->logfile_path,"");
    C->logLevel = LOG_DEBUG;
    C->last = true;
}

void initProxyConfig(ProxyConfig *PC){
    memset(PC,0,sizeof(ProxyConfig));
}

char* trim(char *line,int size){
    char* rline = (char *)malloc(size+1);
    memset(rline,0,size);
    int n = 0;
    for(int i = 0; i < size; i++){
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n' && line[i] != '\xff'){
            rline[n] = line[i];
            n++;
        }
    }
    rline[n] = '\0';
    return rline;
}

void parse_config(char *path, Config *common,bool method){
    initConfig(common);
    FILE *fp;
    if(method){
        fp = fopen(path,"r");
    }else{
        fp = tmpfile();
        fputs(path,fp);
        rewind(fp);
    }
    char* line = (char *)malloc(1024);
    memset(line,1,1024);
    int n = 0;
    while(fgets(line,1024,fp) != NULL){
        char *rline = trim(line,strlen(line));
        strcpy(line,rline);
        kv *KV = pkv(line);
        if(line[0] == '#' || line[0] == ';' || line[0] == '\n'){
            continue;
        }else if (line[0] == '['){
            if (n > 0){
                LastConfig *lc = getLast(common);
                if(lc->last == true){
                    lc->PC->last = false;
                    lc->PC->next = (ProxyConfig *)malloc(sizeof(ProxyConfig));
                    initProxyConfig(lc->PC->next);
                    lc->PC->next->last = true;
                }else{
                    common->next = (ProxyConfig *)malloc(sizeof(ProxyConfig));
                    initProxyConfig(common->next);
                    common->last = false;
                    common->next->last = true;
                }
            }
            LastConfig *lc1 = getLast(common);
            char *end = strchr(line,']');
            if (!end) continue;
            *end = '\0';
            if(lc1->last == true){
                strcpy(lc1->PC->name,line+1);
            }else{
                strcpy(common->name,line+1);
            }
            n++;
        }else if(strcasecmp(KV->key,"bind_ip") == 0 | strcasecmp(KV->key,"server_addr") == 0){
            strcpy(common->addr,KV->value);
        }else if (strcasecmp(KV->key,"bind_port") == 0 | strcasecmp(KV->key,"server_port") == 0){
            common->port = atoi(KV->value);
        }else if (strcasecmp(KV->key,"use_encryption") == 0){
            if(KV->value == "true" && KV->value == "True" && KV->value == "TRUE" && KV->value == "1"){
                common->use_encryption = true;
            }
        }else if (strcasecmp(KV->key,"encryption_key") == 0){
            strcpy(common->encryption_key,KV->value);
        }else if (strcasecmp(KV->key,"auth_token") == 0){
            strcpy(common->auth_token,KV->value);
        }else if (strcasecmp(KV->key,"loglevel") == 0){
            switch(atoi(KV->value)){
                case 0:
                    common->logLevel = LOG_DEBUG;
                    break;
                case 1:
                    common->logLevel = LOG_INFO;
                    break;
                case 2:
                    common->logLevel = LOG_WARN;
                    break;
                case 3:
                    common->logLevel = LOG_ERROR;
                    break;
                default:
                    common->logLevel = LOG_DEBUG;
                    break;
            }
        }else if (strcasecmp(KV->key,"logfile") == 0){
            strcpy(common->logfile_path,KV->value);
        }else if (strcasecmp(KV->key,"type") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                if(strcasecmp(KV->value,"tcp") == 0){
                    lc->PC->type = PROXY_TCP;
                }else if(strcasecmp(KV->value,"stcp") == 0){
                    lc->PC->type = PROXY_STCP;
                }else if (strcasecmp(KV->value,"udp") == 0){
                    lc->PC->type = PROXY_UDP;
                }else if (strcasecmp(KV->value,"sudp") == 0){
                    lc->PC->type = PROXY_SUDP;
                }else if(strcasecmp(KV->value,"socks5") == 0 ){
                    lc->PC->type = PROXY_SOCKS5;
                }
            }
        }else if (strcasecmp(KV->key,"local_port") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                lc->PC->local_port = atoi(KV->value);
            }
        }else if (strcasecmp(KV->key,"local_port") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                lc->PC->local_port = atoi(KV->value);
            }
        }else if (strcasecmp(KV->key,"remote_port") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                lc->PC->remote_port = atoi(KV->value);
            }
        }else if (strcasecmp(KV->key,"username") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                strcpy(lc->PC->username,KV->value);
            }
        }else if (strcasecmp(KV->key,"password") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                strcpy(lc->PC->password,KV->value);
            }
        }else if (strcasecmp(KV->key,"servertype") == 0){
            LastConfig *lc = getLast(common);
            if (lc->last == true){
                strcpy(lc->PC->servertype,KV->value);
            }
        }
    }
    // printConfig(&common);
    fclose(fp);
}

LastConfig *getLast(Config *common){
    bool tmp = common->last;
    struct ProxyConfig *p = common->next;
    if(tmp == true){
        common->last = true;
        goto label1;
    }
    while(p->last != true){
        p = p->next;
    }
    label1:
    LastConfig *lc = (LastConfig *)malloc(sizeof(LastConfig));
    lc->PC = p;
    lc->C = common;

    if (p != NULL){
        lc->last = true;
    }else{
        lc->last = false;
    }
    return lc;
}

kv *pkv(char *line){ 
    kv *dic = (kv *)malloc(sizeof(kv));
    char *key = strchr(line,'=');
    if(!key){
        strcpy(dic->key,"");
        strcpy(dic->value,"");
        return dic;
    }
    strncpy(dic->key,line,key-line);
    dic->key[key-line] = '\0';
    strcpy(dic->value,key+1);
    // printf("Parse: %s = %s\n",dic->key,dic->value);
    return dic;
}

void printConfig(Config *p){
    printf("[%s]\n",p->name);
    printf("bind_ip = %s\n",p->addr);
    printf("bind_port = %d\n",p->port);
    printf("use_encryption = %s\n",p->use_encryption?"False":"True");
    printf("encryption_key = %s\n",p->encryption_key);
    printf("auth_token = %s\n",p->auth_token);
    for(ProxyConfig *pc = p->next;pc!=NULL;pc=pc->next){
        printf("[%s]\n",pc->name);
        printf("type = %s\n",pc->type==PROXY_TCP?"tcp":pc->type==PROXY_STCP?"stcp":pc->type==PROXY_UDP?"udp":pc->type==PROXY_SUDP?"sudp":pc->type==PROXY_SOCKS5?"socks5":"unknown");
        printf("username = %s\n",pc->username);
        printf("password = %s\n",pc->password);
        printf("local_port = %d\n",pc->local_port);
        printf("remote_port = %d\n",pc->remote_port);
        printf("server_type = %s\n",pc->servertype);
    }
}


void freeConfig(Config *p){
    ProxyConfig **pc = &p->next;
    free(p);
    ProxyConfig *tmp;
    while(pc != NULL){
        tmp = *pc;
        *pc = tmp->next;
        free(tmp);
    }
}