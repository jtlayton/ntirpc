/*
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
 * \file    nfs_read_conf.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/12/07 14:28:00 $
 * \version $Revision: 1.10 $
 * \brief   This file that contain the routine required for parsing the NFS specific configuraion file.
 *
 * nfs_read_conf.c : This file that contain the routine required for parsing the NFS specific configuraion file.
 *
 * $Header: /cea/S/home/cvs/cvs/SHERPA/BaseCvs/GANESHA/src/support/nfs_read_conf.c,v 1.10 2005/12/07 14:28:00 deniel Exp $
 *
 * $Log: nfs_read_conf.c,v $
 * Revision 1.10  2005/12/07 14:28:00  deniel
 * Support of stats via stats_thread was added
 *
 * Revision 1.9  2005/11/30 15:41:15  deniel
 * Added IP/stats conf
 *
 * Revision 1.8  2005/11/29 13:38:18  deniel
 * bottlenecked ip_stats
 *
 * Revision 1.7  2005/11/28 17:03:03  deniel
 * Added CeCILL headers
 *
 * Revision 1.6  2005/11/08 15:22:24  deniel
 * WildCard and Netgroup entry for exportlist are now supported
 *
 * Revision 1.5  2005/10/10 14:27:54  deniel
 * mnt_Mnt does not create root entries in Cache inode any more. This is done before the first request
 * once the export list is read the first time .
 *
 * Revision 1.4  2005/10/07 07:34:00  deniel
 * Added default parameters support to be able to manage 'simplified' config file
 *
 * Revision 1.3  2005/08/11 12:37:28  deniel
 * Added statistics management
 *
 * Revision 1.2  2005/08/03 13:14:00  deniel
 * memset to zero before building the filehandles
 *
 * Revision 1.1  2005/08/03 06:57:54  deniel
 * Added a libsupport for miscellaneous service functions
 *
 * Revision 1.1  2005/07/18 14:12:45  deniel
 * Fusion of the dirrent layers in progreess via the implementation of mount protocol
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifdef _SOLARIS
#include "solaris_port.h"
#endif


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>  /* for having FNDELAY */
#ifdef _USE_GSSRPC
#include <gssrpc/types.h>
#include <gssrpc/rpc.h>
#include <gssrpc/auth.h>
#include <gssrpc/pmap_clnt.h>
#else
#include <rpc/types.h>
#include <rpc/rpc.h>
#include <rpc/auth.h>
#include <rpc/pmap_clnt.h>
#endif
#include "log_functions.h"
#include "stuff_alloc.h"
#include "fsal.h"
#include "nfs23.h"
#include "nfs4.h"
#include "mount.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "cache_content.h"
#include "nfs_file_handle.h"
#include "nfs_exports.h"
#include "nfs_tools.h"
#include "nfs_proto_functions.h"
#include "nfs_dupreq.h"
#include "config_parsing.h"


/**
 *
 * nfs_read_worker_conf: read the configuration ite; for the worker theads.
 * 
 * Reads the configuration ite; for the worker theads.
 * 
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok, -1 if failed,1 is stanza is not there
 *
 */
int nfs_read_worker_conf( config_file_t            in_config,
                          nfs_worker_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;
  
  /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL  )
    return CACHE_INODE_INVALID_ARGUMENT ;
  
  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_WORKER ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_WORKER  ) ; */
      return  1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }
  
  var_max = config_GetNbItems( block );
  
  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n", 
                  var_index, CONF_LABEL_NFS_WORKER ); 
          return CACHE_INODE_INVALID_ARGUMENT ;
        }
      
      if( !strcasecmp( key_name, "Pending_Job_Prealloc" ) )
        {
          pparam->nb_pending_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_Before_GC" ) )
        {
          pparam->nb_before_gc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_DupReq_Prealloc" ) )
        {
          pparam->nb_dupreq_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_DupReq_Before_GC" ) )
        {
          pparam->nb_dupreq_before_gc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_Client_Id_Prealloc" ) )
        {
          pparam->nb_client_id_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_IP_Stats_Prealloc" ) )
        {
          pparam->nb_ip_stats_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "LRU_Pending_Job_Prealloc_PoolSize" ) )
        {
          pparam->lru_param.nb_entry_prealloc  = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "LRU_DupReq_Prealloc_PoolSize" ) )
        {
          pparam->lru_dupreq.nb_entry_prealloc = atoi( key_value ) ;
        }
      else
        {
          fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                   key_name, CONF_LABEL_NFS_WORKER ) ;
          return -1 ;
        }
      
    }
  

  return 0 ;
} /* nfs_read_worker_conf */



/**
 *
 * nfs_read_core_conf: read the configuration ite; for the worker theads.
 * 
 * Reads the configuration ite; for the worker theads.
 * 
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok, -1 if failed, 1 is stanza is not there.
 *
 */
int nfs_read_core_conf( config_file_t          in_config,
                        nfs_core_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;
  
  /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL  )
    return CACHE_INODE_INVALID_ARGUMENT ;
  
  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_CORE ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_CORE  ) ; */
      return  1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }
  
  var_max = config_GetNbItems( block );
  
 
  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_NFS_CORE ) ;
          return CACHE_INODE_INVALID_ARGUMENT ;
        }

      if( !strcasecmp( key_name, "Nb_Worker" ) )
        {
          pparam->nb_worker = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_MaxConcurrentGC" ) ) 
	{
	  pparam->nb_max_concurrent_gc = atoi( key_value ) ;
	}
      else if( !strcasecmp( key_name, "DupReq_Expiration" ) )
        {
          pparam->expiration_dupreq = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Use_NFS_Commit" ) )
        {
          pparam->use_nfs_commit = StrToBoolean( key_value ) ;
        }
      else if( !strcasecmp( key_name, "NFS_Port" ) )
        {
          pparam->nfs_port = (unsigned short)atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Drop_IO_Errors" ) ) 
	{
          pparam->drop_io_errors = StrToBoolean( key_value ) ;
	}
      else if( !strcasecmp( key_name, "Drop_Inval_Errors" ) ) 
	{
          pparam->drop_inval_errors = StrToBoolean( key_value ) ;
	}
      else if( !strcasecmp( key_name, "MNT_Port" ) )
        {
          pparam->mnt_port = (unsigned short)atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "NFS_Program" ) )
        {
          pparam->nfs_program = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "MNT_Program" ) )
        {
          pparam->mnt_program = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Core_Dump_Size" ) ) 
        {
          pparam->core_dump_size = atol( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Nb_Max_Fd" ) ) 
	{
          pparam->nb_max_fd = atoi( key_value ) ;
	}
      else if( !strcasecmp( key_name, "Stats_File_Path" ) )
        {
          strncpy( pparam->stats_file_path, key_value, MAXPATHLEN ) ;
        }
      else if( !strcasecmp( key_name, "Stats_Update_Delay" ) )
        {
          pparam->stats_update_delay = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Dump_Stats_Per_Client" ) )
	{
	  pparam->dump_stats_per_client = StrToBoolean( key_value ) ;
	}
      else if( !strcasecmp( key_name, "Stats_Per_Client_Directory" ) )
	{
	  strncpy( pparam->stats_per_client_directory, key_value, MAXPATHLEN ) ;
	}
      else
        {
          fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                   key_name, CONF_LABEL_NFS_CORE ) ;
          return -1 ;
        }
      
    }
  

  return 0 ;
} /* nfs_read_core_conf */

/**
 *
 * nfs_read_dupreq_hash_conf: reads the configuration for the hash in Duplicate Request layer.
 * 
 * Reads the configuration for the hash in Duplicate Request layer
 * 
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_dupreq_hash_conf( config_file_t                in_config,
                               nfs_rpc_dupreq_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_DUPREQ ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_DUPREQ ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }
  
  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_NFS_DUPREQ ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) ) 
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_NFS_DUPREQ ) ;
           return -1 ;
        }
    }
  

  return  0 ;
} /* nfs_read_dupreq_hash_conf */


/**
 *
 * nfs_read_ip_name_conf: reads the configuration for the IP/name.
 * 
 * Reads the configuration for the IP/name.
 * 
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_ip_name_conf( config_file_t             in_config,
                           nfs_ip_name_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_IP_NAME ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_IP_NAME ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }
  
  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_NFS_IP_NAME ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) ) 
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Expiration_Time" ) )
        {
          pparam->expiration_time = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Map" ) ) 
        {
	   strncpy( pparam->mapfile, key_value, MAXPATHLEN ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_NFS_IP_NAME ) ;
           return -1 ;
        }
    }
  

  return  0 ;
} /* nfs_read_ip_name_conf */

/**
 *
 * nfs_read_ip_name_conf: reads the configuration for the Client/ID Cache
 *
 * Reads the configuration for the Client/ID Cache
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_client_id_conf( config_file_t               in_config,
                             nfs_client_id_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_CLIENT_ID ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_CLIENT_ID ) ; */
      return 1 ;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_CLIENT_ID ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) )
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_CLIENT_ID ) ;
           return -1 ;
        }
    }


  return  0 ;
} /* nfs_client_id_conf */

/**
 *
 * nfs_read_ip_name_conf: reads the configuration for the Client/ID Cache
 *
 * Reads the configuration for the Client/ID Cache
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_state_id_conf( config_file_t               in_config,
                            nfs_state_id_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_STATE_ID ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_STATE_ID ) ; */
      return 1 ;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_STATE_ID ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) )
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_STATE_ID ) ;
           return -1 ;
        }
    }


  return  0 ;
} /* nfs_state_id_conf */

#ifdef _USE_NFS4_1
int nfs_read_session_id_conf( config_file_t                in_config,
                              nfs_session_id_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_SESSION_ID ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_STATE_ID ) ; */
      return 1 ;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_SESSION_ID ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) )
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_SESSION_ID ) ;
           return -1 ;
        }
    }


  return  0 ;
} /* nfs_session_id_conf */
#endif

/**
 *
 * nfs_read_uidmap_conf: reads the configuration for the UID_MAPPER Cache
 *
 * Reads the configuration for the UID_MAPPER Cache
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_uidmap_conf( config_file_t                 in_config,
                          nfs_idmap_cache_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_UID_MAPPER ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_CLIENT_ID ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_UID_MAPPER ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) )
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Map" ) )
        {
          strncpy( pparam->mapfile, key_value, MAXPATHLEN ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_UID_MAPPER ) ;
           return -1 ;
        }
    }

  return  0 ;
} /* nfs_read_uidmap_conf */


/**
 *
 * nfs_read_gidmap_conf: reads the configuration for the GID_MAPPER Cache
 *
 * Reads the configuration for the GID_MAPPER Cache
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok,  -1 if not, 1 is stanza is not there.
 *
 */
int nfs_read_gidmap_conf( config_file_t                 in_config,
                          nfs_idmap_cache_parameter_t * pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_GID_MAPPER ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_CLIENT_ID ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_GID_MAPPER ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Index_Size" ) )
        {
          pparam->hash_param.index_size = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Alphabet_Length" ) )
        {
          pparam->hash_param.alphabet_length = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Prealloc_Node_Pool_Size" ) )
        {
          pparam->hash_param.nb_node_prealloc = atoi( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Map" ) )
        {
	   strncpy( pparam->mapfile, key_value, MAXPATHLEN ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_GID_MAPPER ) ;
           return -1 ;
        }
    }

  return  0 ;
} /* nfs_read_gidmap_conf */


/**
 *
 * nfs_read_krb5_conf: read the configuration for krb5 stuff
 *
 * Read the configuration for krb5 stuff.
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok, -1 if failed,1 is stanza is not there
 *
 */
int nfs_read_krb5_conf( config_file_t            in_config,
                        nfs_krb5_parameter_t *   pparam )
{ 
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_KRB5 ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_KRB5 ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }

  var_max = config_GetNbItems( block );

  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
      
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_NFS_KRB5 ) ;
          return  -1 ;
        }
    
      if( !strcasecmp( key_name, "PrincipalName" ) ) 
	{
	  strncpy( pparam->principal, key_value, MAXNAMLEN ) ;
	}
      else if( !strcasecmp( key_name, "KeytabPath" ) ) 
	{
	  strncpy( pparam->keytab, key_value, MAXPATHLEN ) ;
	}  
      else if( !strcasecmp( key_name, "Active_krb5" ) )
        {
          pparam->active_krb5 = StrToBoolean( key_value ) ;
        }
      else
	{
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_NFS_KRB5 ) ;
           return -1 ;
        }
      }

  return 0 ;
} /* nfs_read_krb5_conf */


/**
 *
 * nfs_read_version4_conf: read the configuration for NFSv4 stuff
 *
 * Read the configuration for NFSv4 stuff.
 *
 * @param in_config [IN] configuration file handle
 * @param pparam [OUT] read parameters
 *
 * @return 0 if ok, -1 if failed,1 is stanza is not there
 *
 */

int nfs_read_version4_conf( config_file_t                in_config,
                            nfs_version4_parameter_t *   pparam )
{
  int     var_max ;
  int     var_index ;
  int     err ;
  char *  key_name ;
  char *  key_value ;
  config_item_t   block;

   /* Is the config tree initialized ? */
  if( in_config == NULL || pparam == NULL )
    return -1 ;

  /* Get the config BLOCK */
  if( ( block = config_FindItemByName( in_config, CONF_LABEL_NFS_VERSION4 ) ) == NULL )
    {
      /* fprintf(stderr, "Cannot read item \"%s\" from configuration file\n", CONF_LABEL_NFS_VERSION4 ) ; */
      return 1 ;
    }
  else if ( config_ItemType(block) != CONFIG_ITEM_BLOCK )
    {
      /* Expected to be a block*/
      return 1;
    }

  var_max = config_GetNbItems( block );
  
  for( var_index = 0 ; var_index < var_max ; var_index++ )
    {
      config_item_t item;
    
      item = config_GetItemByIndex( block, var_index );
    
      /* Get key's name */
      if( ( err = config_GetKeyValue( item,
                                      &key_name,
                                      &key_value ) ) != 0 )
        {
          fprintf(stderr, "Error reading key[%d] from section \"%s\" of configuration file.\n",
                  var_index, CONF_LABEL_NFS_VERSION4 ) ;
          return  -1 ;
        }

      if( !strcasecmp( key_name, "Lease_Lifetime" ) )
        {
          pparam->lease_lifetime = atoi( key_value ) ; 
        }
      else if( !strcasecmp( key_name, "DomainName" ) )
        {
	   strncpy( pparam->domainname, key_value, MAXNAMLEN ) ;
        }
      else if( !strcasecmp( key_name, "IdmapConf" ) )
        {
	   strncpy( pparam->idmapconf, key_value, MAXPATHLEN ) ;
        }
      else if( !strcasecmp( key_name, "FH_Expire" ) ) 
        {
	  pparam->fh_expire = StrToBoolean( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Returns_ERR_FH_EXPIRED" ) )
        {
	  pparam->returns_err_fh_expired = StrToBoolean( key_value ) ;
        }
      else if( !strcasecmp( key_name, "Use_OPEN_CONFIRM" ) )
        {
	  pparam->use_open_confirm = StrToBoolean( key_value ) ;
        }
      else
        {
           fprintf( stderr,
                   "Unknown or unsettable key: %s (item %s)\n",
                    key_name, CONF_LABEL_NFS_VERSION4 ) ;
           return -1 ;
        }
      }

  return 0 ;
} /* nfs_read_version4_conf */

/**
 *
 * Print_param_in_log : prints the nfs worker parameter structure into the logfile
 *
 * prints the nfs worker parameter structure into the logfile
 *
 * @param pparam Pointer to the nfs worker parameter
 * 
 * @return none (void function)
 *
 */
void Print_param_worker_in_log( nfs_worker_parameter_t * pparam )
{
  DisplayLog( "NFS PARAM : worker_param.lru_param.nb_entry_prealloc = %d", 
              pparam->lru_param.nb_entry_prealloc) ;
  DisplayLog( "NFS PARAM : worker_param.nb_pending_prealloc = %d",  pparam->nb_pending_prealloc ) ;
  DisplayLog( "NFS PARAM : worker_param.nb_before_gc = %d",  pparam->nb_before_gc ) ;
} /* Print_param_worker_in_log */

/**
 *
 * Print_param_in_log : prints the nfs parameter structure into the logfile
 *
 * prints the nfs parameter structure into the logfile
 *
 * @param pparam Pointer to the nfs parameter
 * 
 * @return none (void function)
 *
 */
void Print_param_in_log( nfs_parameter_t * pparam )
{
  DisplayLog( "NFS PARAM : core_param.nb_worker = %d", pparam->core_param.nb_worker ) ;
  Print_param_worker_in_log( &(pparam->worker_param) ) ;
} /* Print_param_in_log */