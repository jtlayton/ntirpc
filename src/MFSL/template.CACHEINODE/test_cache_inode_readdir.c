/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * Ce logiciel est un serveur implementant le protocole NFS.
 *
 * Ce logiciel est r�gi par la licence CeCILL soumise au droit fran�ais et
 * respectant les principes de diffusion des logiciels libres. Vous pouvez
 * utiliser, modifier et/ou redistribuer ce programme sous les conditions
 * de la licence CeCILL telle que diffus�e par le CEA, le CNRS et l'INRIA
 * sur le site "http://www.cecill.info".
 *
 * En contrepartie de l'accessibilit� au code source et des droits de copie,
 * de modification et de redistribution accord�s par cette licence, il n'est
 * offert aux utilisateurs qu'une garantie limit�e.  Pour les m�mes raisons,
 * seule une responsabilit� restreinte p�se sur l'auteur du programme,  le
 * titulaire des droits patrimoniaux et les conc�dants successifs.
 *
 * A cet �gard  l'attention de l'utilisateur est attir�e sur les risques
 * associ�s au chargement,  � l'utilisation,  � la modification et/ou au
 * d�veloppement et � la reproduction du logiciel par l'utilisateur �tant
 * donn� sa sp�cificit� de logiciel libre, qui peut le rendre complexe �
 * manipuler et qui le r�serve donc � des d�veloppeurs et des professionnels
 * avertis poss�dant  des  connaissances  informatiques approfondies.  Les
 * utilisateurs sont donc invit�s � charger  et  tester  l'ad�quation  du
 * logiciel � leurs besoins dans des conditions permettant d'assurer la
 * s�curit� de leurs syst�mes et ou de leurs donn�es et, plus g�n�ralement,
 * � l'utiliser et l'exploiter dans les m�mes conditions de s�curit�.
 *
 * Le fait que vous puissiez acc�der � cet en-t�te signifie que vous avez
 * pris connaissance de la licence CeCILL, et que vous en avez accept� les
 * termes.
 *
 * ---------------------
 *
 * Copyright CEA/DAM/DIF (2005)
 *  Contributor: Philippe DENIEL  philippe.deniel@cea.fr
 *               Thomas LEIBOVICI thomas.leibovici@cea.fr
 *
 *
 * This software is a server that implements the NFS protocol.
 * 
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * ---------------------------------------
 */

/**
 * \file    test_cache_inode_lookup.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/11/28 17:02:28 $
 * \version $Revision: 1.9 $
 * \brief   Program for testing cache_inode / lookup functions. 
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BuddyMalloc.h"
#include "fsal.h"

#include "cache_inode.h"
#include "LRU_List.h"
#include "log_functions.h"
#include "err_fsal.h"
#include "err_cache_inode.h"
#include "stuff_alloc.h"
#include <unistd.h>             /* for using gethostname */
#include <stdlib.h>             /* for using exit */
#include <strings.h>
#include <sys/types.h>

#define HPSS_SSM "hpss_ssm"
#define HPSS_KEYTAB "/krb5/hpssserver.keytab"

int lru_entry_to_str(LRU_data_t data, char *str)
{
  cache_entry_t *pentry = NULL;

  pentry = (cache_entry_t *) data.pdata;

  return sprintf(str, "Pentry: Addr %p, state=%d", pentry,
                 pentry->internal_md.valid_state);
}                               /* lru_entry_to_str */

int lru_clean_entry(LRU_entry_t * entry, void *adddata)
{
  return 0;
}                               /* lru_clean_entry */

/**
 *
 * main: test main function.
 *
 * test main function.
 *
 * @param argc [IN] number of command line arguments.
 * @param argv [IN] arguments set on the command line.
 *
 * @return an integer sent back to the calling shell.
 *
 */

main(int argc, char *argv[])
{
  log_t log_desc_fsal = LOG_INITIALIZER;
  log_t log_desc_cache = LOG_INITIALIZER;
  char localmachine[256];
  desc_log_stream_t voie_fsal;
  desc_log_stream_t voie_cache;

  cache_inode_client_t client;
  LRU_parameter_t lru_param;
  LRU_status_t lru_status;
  cache_inode_fsal_data_t fsdata;

  fsal_status_t status;
  fsal_parameter_t init_param;
  fsal_name_t name;
  fsal_path_t path;
  fsal_attrib_mask_t mask;
  fsal_path_t pathroot;
  fsal_attrib_list_t attribs;
  fsal_handle_t root_handle;

  cache_inode_endofdir_t eod_met;
  cache_inode_dir_entry_t dirent_array[100];
  cache_inode_dir_entry_t dirent_array_loop[5];
  unsigned int nbfound;

  unsigned int begin_cookie = 0;
  hash_buffer_t key, value;

  uid_t uid;
  fsal_cred_t cred;

  cache_inode_status_t cache_status;
  cache_inode_parameter_t cache_param;
  cache_inode_client_parameter_t cache_client_param;

  hash_table_t *ht = NULL;
  fsal_attrib_list_t attrlookup;
  cache_entry_t *cache_entry_root = NULL;
  cache_entry_t *cache_entry_lookup = NULL;
  cache_entry_t *cache_entry_lookup2 = NULL;
  cache_entry_t *cache_entry_lookup3 = NULL;
  cache_entry_t *cache_entry_lookup4 = NULL;
  cache_entry_t *cache_entry_lookup5 = NULL;
  cache_entry_t *cache_entry_lookup6 = NULL;
  cache_entry_t *cache_entry_dircont = NULL;

  cache_inode_gc_policy_t gcpol;

  char *configfile = argv[1];
  int i = 0;
  int rc = 0;

  /* Init the Buddy System allocation */
  if ((rc = BuddyInit(NULL)) != BUDDY_SUCCESS)
    {
      fprintf(stderr, "Error while Initing the Buddy system allocator");
      exit(1);
    }

  /* init debug */
  SetNamePgm("test_cache_inode");
  SetNameFunction("main");
  SetNameFileLog("/dev/tty");

#if defined( _USE_GHOSTFS )
  if (argc != 2)
    {
      fprintf(stderr, "Please set the configuration file as parameter\n");
      exit(1);
    }
#endif

  /* Obtention du nom de la machine */
  if (gethostname(localmachine, sizeof(localmachine)) != 0)
    {
      DisplayErrorLog(ERR_SYS, ERR_GETHOSTNAME, errno);
      exit(1);
    } else
    SetNameHost(localmachine);

  InitDebug(NIV_FULL_DEBUG);
  AddFamilyError(ERR_FSAL, "FSAL related Errors", tab_errstatus_FSAL);
  AddFamilyError(ERR_CACHE_INODE, "FSAL related Errors", tab_errstatus_cache_inode);

  /* creating log */
  voie_fsal.fd = fileno(stdout);
  AddLogStreamJd(&log_desc_fsal, V_FD, voie_fsal, NIV_FULL_DEBUG, SUP);

  voie_cache.fd = fileno(stdout);
  AddLogStreamJd(&log_desc_cache, V_FD, voie_cache, NIV_FULL_DEBUG, SUP);

  DisplayLogJd(log_desc_cache, "Starting the test");
  DisplayLogJd(log_desc_cache, "-----------------");

#if defined( _USE_GHOSTFS )

  if (FSAL_IS_ERROR(status = FSAL_str2path(configfile,
                                           strlen(configfile) + 1,
                                           &(init_param.fs_specific_info.
                                             definition_file))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
    }
#elif defined( _USE_HPSS )

  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, Flags);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DebugValue);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, TransferType);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, NumRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, BusyDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, BusyRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, TotalDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, GKTotalDelay);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, LimitedRetries);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, MaxConnections);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, ReuseDataConnections);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, UsePortRange);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, RetryStageInp);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DMAPWriteUpdates);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, ServerName);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DescName);

  init_param.fs_specific_info.behaviors.PrincipalName = FSAL_INIT_FORCE_VALUE;
  strncpy(init_param.fs_specific_info.hpss_config.PrincipalName,
          HPSS_SSM, HPSS_MAX_PRINCIPAL_NAME);

  init_param.fs_specific_info.behaviors.KeytabPath = FSAL_INIT_FORCE_VALUE;
  strncpy(init_param.fs_specific_info.hpss_config.KeytabPath,
          HPSS_KEYTAB, HPSS_MAX_PATH_NAME);

  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, DebugPath);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, HostName);
  FSAL_SET_INIT_DEFAULT(init_param.fs_specific_info, RegistrySiteName);

#endif
  /* 2-common info (default) */
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxfilesize);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxlink);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxnamelen);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxpathlen);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, no_trunc);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, chown_restricted);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, case_insensitive);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, case_preserving);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, fh_expire_type);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, link_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, symlink_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, named_attr);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, unique_handles);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, lease_time);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, acl_support);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, cansettime);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, homogenous);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxread);
  FSAL_SET_INIT_DEFAULT(init_param.fs_common_info, maxwrite);

  /* 3- fsal info */
  init_param.fsal_info.log_outputs = log_desc_fsal;

  /* Init */
  if (FSAL_IS_ERROR(status = FSAL_Init(&init_param)))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
    }

  /* getting creds */
  uid = getuid();

  if (FSAL_IS_ERROR(status = FSAL_GetUserCred(uid, NULL, &cred)))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
    }

  /* Init of the cache inode module */
  cache_param.hparam.index_size = 31;
  cache_param.hparam.alphabet_length = 10;      /* Buffer seen as a decimal polynom */
  cache_param.hparam.nb_node_prealloc = 100;
  cache_param.hparam.hash_func_key = cache_inode_fsal_hash_func;
  cache_param.hparam.hash_func_rbt = cache_inode_fsal_rbt_func;
  cache_param.hparam.compare_key = cache_inode_compare_key_fsal;
  cache_param.hparam.key_to_str = display_key;
  cache_param.hparam.val_to_str = display_value;

  if ((ht = cache_inode_init(cache_param, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error %d while init hash ", cache_status);
    } else
    DisplayLogJd(log_desc_cache, "Hash Table address = %p", ht);

  /* We need a cache_client to acces the cache */
  cache_client_param.log_outputs = log_desc_cache;
  cache_client_param.attrmask =
      FSAL_ATTRS_MANDATORY | FSAL_ATTR_MTIME | FSAL_ATTR_CTIME | FSAL_ATTR_ATIME;
  cache_client_param.nb_prealloc_entry = 1000;
  cache_client_param.nb_pre_dir_data = 200;
  cache_client_param.nb_pre_parent = 1200;
  cache_client_param.nb_pre_state_v4 = 100;
  cache_client_param.nb_pre_async_op_desc = 100;

  cache_client_param.lru_param.nb_entry_prealloc = 1000;
  cache_client_param.lru_param.entry_to_str = lru_entry_to_str;
  cache_client_param.lru_param.clean_entry = lru_clean_entry;

  cache_client_param.grace_period_attr = 0;
  cache_client_param.grace_period_link = 0;

  /* Init the cache_inode client */
  if (cache_inode_client_init(&client, cache_client_param, 0) != 0)
    exit(1);

  /* Getting the root of the FS */
  if ((FSAL_IS_ERROR(status = FSAL_str2path("/", 2, &pathroot))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  attribs.asked_attributes = cache_client_param.attrmask;
  if ((FSAL_IS_ERROR(status = FSAL_lookupPath(pathroot, &cred, &root_handle, &attribs))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }
  fsdata.cookie = 0;
  fsdata.handle = root_handle;

  /* Cache the root of the FS */
  if ((cache_entry_root =
       cache_inode_make_root(&fsdata, 1, ht, &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't init fs's root");
      exit(1);
    }

  cache_inode_print_dir(cache_entry_root);

  /* Test readdir */
  if (cache_inode_readdir(cache_entry_root,
                          0,
                          100,
                          &nbfound,
                          &eod_met,
                          dirent_array,
                          ht, &client, &cred, &cache_status) != CACHE_INODE_SUCCESS)
    {
      DisplayLogJd(log_desc_cache, "Error: cache_inode_readdir failed\n");
      exit(1);
    }

  DisplayLogJd(log_desc_cache, "Readdir nbfound=%d, eod_met=%d", nbfound, eod_met);
  for (i = 0; i < nbfound; i++)
    DisplayLogJd(log_desc_cache, "dirent_array[%d] ==> %s | %p", i,
                 dirent_array[i].name.name, dirent_array[i].pentry);

  cache_inode_print_dir(cache_entry_root);

  /* looping on readir */
  DisplayLogJd(log_desc_cache, "Loop directory in several pass");

  eod_met = TO_BE_CONTINUED;
  begin_cookie = 0;

  do
    {

      if (cache_inode_readdir(cache_entry_root,
                              begin_cookie,
                              2,
                              &nbfound,
                              &eod_met,
                              dirent_array_loop,
                              ht, &client, &cred, &cache_status) != CACHE_INODE_SUCCESS)
        {
          printf("Error: cache_inode_readdir failed: %d\n", cache_status);
          exit(1);
        }

      for (i = 0; i < nbfound; i++)
        DisplayLogJd(log_desc_cache, " ==> %s | %p", dirent_array_loop[i].name.name,
                     dirent_array_loop[i].pentry);

      begin_cookie += nbfound;

    }
  while (eod_met == TO_BE_CONTINUED);

  DisplayLogJd(log_desc_cache, "---------------------------------");

  cache_inode_print_dir(cache_entry_root);

  /* A lookup in the root fsal */
  if ((FSAL_IS_ERROR(status = FSAL_str2name("cea", 10, &name))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if ((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't lookup");
      exit(1);
    }

  /* Lookup a second time (entry should now be cached) */
  if ((cache_entry_lookup2 = cache_inode_lookup(cache_entry_root,
                                                name,
                                                &attrlookup,
                                                ht,
                                                &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_fsal, "Error: can't lookup");
      exit(1);
    }

  if (cache_entry_lookup2 != cache_entry_lookup)
    {
      printf("Error: lookup results should be the same\n");
      exit(1);
    }

  /* A lookup in the root fsal */
  if ((FSAL_IS_ERROR(status = FSAL_str2name("log", 10, &name))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if ((cache_entry_lookup3 = cache_inode_lookup(cache_entry_root,
                                                name,
                                                &attrlookup,
                                                ht,
                                                &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't lookup");
      exit(1);
    }

  if ((cache_entry_lookup4 = cache_inode_lookup(cache_entry_root,
                                                name,
                                                &attrlookup,
                                                ht,
                                                &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't lookup");
      exit(1);
    }

  if (cache_entry_lookup3 != cache_entry_lookup4)
    {
      printf("Error: lookup results should be the same\n");
      exit(1);
    }

  /* A lookup in the root fsal */
  cache_inode_print_dir(cache_entry_root);

  /* A lookup in the root fsal */
  if ((FSAL_IS_ERROR(status = FSAL_str2name("cea", 10, &name))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if ((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't lookup");
      exit(1);
    }

  /* A lookup in the root fsal */
  if ((FSAL_IS_ERROR(status = FSAL_str2name("log", 10, &name))))
    {
      DisplayErrorJd(log_desc_fsal, ERR_FSAL, status.major, status.minor);
      exit(1);
    }

  if ((cache_entry_lookup = cache_inode_lookup(cache_entry_root,
                                               name,
                                               &attrlookup,
                                               ht,
                                               &client, &cred, &cache_status)) == NULL)
    {
      DisplayLogJd(log_desc_cache, "Error: can't lookup");
      exit(1);
    }

  DisplayLogJd(log_desc_cache, "---------------------------------");

  /* The end of all the tests */
  DisplayLogJd(log_desc_cache, "All tests exited successfully");

  exit(0);
}                               /* main */