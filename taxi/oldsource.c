/* crea SO_SOURCE processi dedicati che si legano ad una cella della mappa  */
    for(i = 0; i < param->so_source; i++){
        switch(fork()){
            case -1:
                ERROR_EXIT
            case 0: /* ----- codice figlio -----  */
                
                /* processo SOURCE si associa ad una cella libera che non sia HOLE */
                source_pos = place_source(city_map);
                sigaction(SIGINT, &sa, NULL);
                
                printf("source pid %ld\n", (long)getpid());
                while(t){
                    
                    /* in attesa del master che autorizza l'avvio della simulazione */
                    sops[0].sem_num = SEM_SOURCE;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                    
                    /* genera richieste taxi con un intervallo variabile tra 1 nsec - 1 sec */
                    sigprocmask(SIG_BLOCK, &my_mask, NULL);
                    treq.tv_nsec = get_random(1, 999999999);
                    if(nanosleep(&treq, &trem) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }                
                    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                    
                    /* preparazione della richiesta */
                    q.start_cell = (long)getpid();
                    /* genera una posizione che sia una cella diversa dall'attuale e non sia hole */
                    q.aim_cell = get_aim_cell(city_map, source_pos);
                
                    if(msgsnd(qid, &q, sizeof(struct request_queue) - sizeof(long), 0) == -1){
                        /* gestire caso coda piena */
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                
                    
                    sigprocmask(SIG_BLOCK, &my_mask, NULL);
                    sops[0].sem_num = SEM_MASTER;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;
                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    
                    }
                    

                    /* ----- SEZIONE CRITICA ----- */
                
                    /* registrazione dell'avvenuta creazione della richiesta */                
                    
                    stat->n_request +=1;      

                    /*printf("Source %ld richiesta partenza %d di arrivo a %ld registrata\n", (long)getpid(), source_pos, q.aim_cell);*/
                    /* ----- FINE SEZIONE CRITITCA ----- */
                    
                    sops[0].sem_num = SEM_MASTER;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;

                    /*sops[1].sem_num = SEM_SOURCE;
                    sops[1].sem_op = 1;
                    sops[1].sem_flg = 0;*/

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                    
                    sops[0].sem_num = SEM_SOURCE;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                }

                exit(EXIT_SUCCESS);
        }
    }