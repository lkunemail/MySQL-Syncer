
#include <rs_config.h>
#include <rs_core.h>
#include <rs_master.h>


static int rs_init_master_conf(rs_master_info_t *mi);

rs_master_info_t *rs_init_master_info(rs_master_info_t *om) 
{
    int                 nl, nd, err;
    rs_master_info_t    *mi;

    nl = 1;
    nd = 1;

    mi = (rs_master_info_t *) malloc(sizeof(rs_master_info_t));

    if(mi == NULL) {
        rs_log_err(rs_errno, "malloc() failed, sizeof(rs_master_into_t)");
        goto free;
    }

    rs_master_info_t_init(mi);

    /* init master conf */
    if(rs_init_master_conf(mi) != RS_OK) {
        goto free;
    }

    /* slab info */
    if(mi->slab_factor <= 1) {
        mi->slab_factor = RS_SLAB_GROW_FACTOR; 
        rs_log_info("slab_facto use default value, %u", mi->slab_factor);
    }

    if(mi->slab_init_size <= 0) {
        mi->slab_init_size = RS_SLAB_INIT_SIZE;
        rs_log_info("slab_init_size use default value, %u", 
                mi->slab_init_size);
    }

    if(mi->slab_mem_size <= 1 * 1024 * 1024) {
        mi->slab_mem_size = RS_SLAB_MEM_SIZE; 
        rs_log_info("slab_mem_size use default value, %u", mi->slab_mem_size);
    }

    if(om != NULL) {

        /* init dump listen */
        if((nl = ((rs_strcmp(mi->listen_addr, om->listen_addr) != 0) || 
                        (mi->listen_port != om->listen_port))))
        {
            rs_log_info("old addr = %s, old port = %d, new addr = %s, "
                    "old port = %d", om->listen_addr, om->listen_port, 
                    mi->listen_addr, mi->listen_port);
        } else {
            rs_log_info("reuse svr_fd");

#if 0
            rs_log_info("stop accept_thread");

            if(om->accept_thread != 0) {
                if((err = pthread_cancel(om->accept_thread)) != 0) {
                    rs_log_err(err, "pthread_cancel() failed, accept_thread");
                } else {
                    if((err = pthread_join(om->accept_thread, NULL)) != 0) {
                        rs_log_err(err, "pthread_join() failed, "
                                "accept_thread");
                    }
                }
            }
#endif 
            mi->svr_fd = om->svr_fd;
            mi->accept_thread = om->accept_thread;

            om->accept_thread = 0; /* NOTICE : don't restop accept thread */
            om->svr_fd = -1; /* NOTICE : reuse fd, don't reclose svr_fd */
        }


        if(!(nd = (rs_strcmp(mi->binlog_idx_file, om->binlog_idx_file) != 0))) 
        {
            if(nl) {
                rs_log_info("free dump threads");
                /* free dump threads */
                rs_free_request_dumps(om->req_dump_info);
            }

            rs_log_info("reuse dump threads");

            mi->req_dump_info = om->req_dump_info;
            om->req_dump_info = NULL; /* NOTICE: don't refree req_dump_info */
        }
    } /* end om */

    if(nl) {
        rs_log_info("start init dump listen");

        /* init dump listen */
        if(rs_dump_listen(mi) != RS_OK) {
            goto free;
        }
    }

    rs_log_info("start accept thread");

    /* start accept thread */
    if((err = pthread_create(&(mi->accept_thread), NULL, 
                    rs_start_accept_thread, mi)) != 0) 
    {
        rs_log_err(err, "pthread_create() failed, accept thread");
        goto free;
    }

    if(nd) {
        rs_log_info("start init dump threads");

        /* init dump threads */
        mi->req_dump_info = (rs_request_dump_info_t *) 
            malloc(sizeof(rs_request_dump_info_t));

        if(mi->req_dump_info == NULL) {
            rs_log_err(rs_errno, "malloc() failed, rs_request_dump_info_t");
            goto free;
        }

        if(rs_init_request_dump(mi->req_dump_info, RS_DUMP_THREADS_NUM) 
                != RS_OK) 
        {
            goto free;
        }
    }

    return mi;

free:

    rs_free_master(mi);

    return NULL;
}

static int rs_init_master_conf(rs_master_info_t *mi)
{

    rs_conf_t *c;

    c = &(mi->conf);

    if(
            (rs_add_conf_kv(c, "listen.addr", &(mi->listen_addr), 
                            RS_CONF_STR) != RS_OK) 
            ||
            (rs_add_conf_kv(c, "listen.port", &(mi->listen_port), 
                            RS_CONF_INT32) != RS_OK) 
            ||
            (rs_add_conf_kv(c, "binlog.index", &(mi->binlog_idx_file), 
                            RS_CONF_STR) != RS_OK) 
            ||
            (rs_add_conf_kv(c, "slab.factor", &(mi->slab_factor), 
                            RS_CONF_DOUBLE) != RS_OK)
            ||
            (rs_add_conf_kv(c, "slab.memsize", &(mi->slab_mem_size), 
                            RS_CONF_UINT32) != RS_OK)
            ||
            (rs_add_conf_kv(c, "slab.initsize", &(mi->slab_init_size), 
                            RS_CONF_UINT32) != RS_OK)
      )
    {
        rs_log_err(0, "rs_add_conf_kv() failed");
        return RS_ERR;
    }

    /* init master conf */
    if(rs_init_conf(c, rs_conf_path, RS_MASTER_MODULE_NAME) != RS_OK) {
        rs_log_err(0, "master conf init failed");
        return RS_ERR;
    }

    return RS_OK;
}
