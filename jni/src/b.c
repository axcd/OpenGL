  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_New_Face( FT_Library   library,
               const char*  pathname,
               FT_Long      face_index,
               FT_Face     *aface )
  {
    FT_Open_Args  args;


    /* test for valid `library' and `aface' delayed to `FT_Open_Face' */
    if ( !pathname )
      return FT_THROW( Invalid_Argument );

    args.flags    = FT_OPEN_PATHNAME;
    args.pathname = (char*)pathname;
    args.stream   = NULL;

    return ft_open_face_internal( library, &args, face_index, aface, 1 );
  }

  
  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Open_Face( FT_Library           library,
                const FT_Open_Args*  args,
                FT_Long              face_index,
                FT_Face             *aface )
  {
    return ft_open_face_internal( library, args, face_index, aface, 1 );
  }


  static FT_Error
  ft_open_face_internal( FT_Library           library,   //&library
                         const FT_Open_Args*  args,      //"ttf"
                         FT_Long              face_index,  //0
                         FT_Face             *aface,       // &face
                         FT_Bool              test_mac_fonts )   //1
  {
    FT_Error     error;
    FT_Driver    driver = NULL;
    FT_Memory    memory = NULL;
    FT_Stream    stream = NULL;
    FT_Face      face   = NULL;
    FT_ListNode  node   = NULL;
    FT_Bool      external_stream;
    FT_Module*   cur;
    FT_Module*   limit;

#ifndef FT_CONFIG_OPTION_MAC_FONTS
    FT_UNUSED( test_mac_fonts );
#endif


#ifdef FT_DEBUG_LEVEL_TRACE
    FT_TRACE3(( "FT_Open_Face: " ));
    if ( face_index < 0 )
      FT_TRACE3(( "Requesting number of faces and named instances\n"));
    else
    {
      FT_TRACE3(( "Requesting face %ld", face_index & 0xFFFFL ));
      if ( face_index & 0x7FFF0000L )
        FT_TRACE3(( ", named instance %ld", face_index >> 16 ));
      FT_TRACE3(( "\n" ));
    }
#endif

    /* test for valid `library' delayed to `FT_Stream_New' */

    if ( ( !aface && face_index >= 0 ) || !args )
      return FT_THROW( Invalid_Argument );

    external_stream = FT_BOOL( ( args->flags & FT_OPEN_STREAM ) &&
                               args->stream                     );

    /* create input stream */
    error = FT_Stream_New( library, args, &stream );
    if ( error )
      goto Fail3;

    memory = library->memory;

    /* If the font driver is specified in the `args' structure, use */
    /* it.  Otherwise, we scan the list of registered drivers.      */
    if ( ( args->flags & FT_OPEN_DRIVER ) && args->driver )
    {
      driver = FT_DRIVER( args->driver );

      /* not all modules are drivers, so check... */
      if ( FT_MODULE_IS_DRIVER( driver ) )
      {
        FT_Int         num_params = 0;
        FT_Parameter*  params     = NULL;


        if ( args->flags & FT_OPEN_PARAMS )
        {
          num_params = args->num_params;
          params     = args->params;
        }

        error = open_face( driver, &stream, external_stream, face_index,
                           num_params, params, &face );
        if ( !error )
          goto Success;
      }
      else
        error = FT_THROW( Invalid_Handle );

      FT_Stream_Free( stream, external_stream );
      goto Fail;
    }
    else
    {
      error = FT_ERR( Missing_Module );

      /* check each font driver for an appropriate format */
      cur   = library->modules;
      limit = cur + library->num_modules;

      for ( ; cur < limit; cur++ )
      {
        /* not all modules are font drivers, so check... */
        if ( FT_MODULE_IS_DRIVER( cur[0] ) )
        {
          FT_Int         num_params = 0;
          FT_Parameter*  params     = NULL;


          driver = FT_DRIVER( cur[0] );

          if ( args->flags & FT_OPEN_PARAMS )
          {
            num_params = args->num_params;
            params     = args->params;
          }

          error = open_face( driver, &stream, external_stream, face_index,
                             num_params, params, &face );
          if ( !error )
            goto Success;

#ifdef FT_CONFIG_OPTION_MAC_FONTS
          if ( test_mac_fonts                                           &&
               ft_strcmp( cur[0]->clazz->module_name, "truetype" ) == 0 &&
               FT_ERR_EQ( error, Table_Missing )                        )
          {
            /* TrueType but essential tables are missing */
            error = FT_Stream_Seek( stream, 0 );
            if ( error )
              break;

            error = open_face_PS_from_sfnt_stream( library,
                                                   stream,
                                                   face_index,
                                                   num_params,
                                                   params,
                                                   aface );
            if ( !error )
            {
              FT_Stream_Free( stream, external_stream );
              return error;
            }
          }
#endif

          if ( FT_ERR_NEQ( error, Unknown_File_Format ) )
            goto Fail3;
        }
      }

    Fail3:
      /* If we are on the mac, and we get an                          */
      /* FT_Err_Invalid_Stream_Operation it may be because we have an */
      /* empty data fork, so we need to check the resource fork.      */
      if ( FT_ERR_NEQ( error, Cannot_Open_Stream )       &&
           FT_ERR_NEQ( error, Unknown_File_Format )      &&
           FT_ERR_NEQ( error, Invalid_Stream_Operation ) )
        goto Fail2;

#if !defined( FT_MACINTOSH ) && defined( FT_CONFIG_OPTION_MAC_FONTS )
      if ( test_mac_fonts )
      {
        error = load_mac_face( library, stream, face_index, aface, args );
        if ( !error )
        {
          /* We don't want to go to Success here.  We've already done   */
          /* that.  On the other hand, if we succeeded we still need to */
          /* close this stream (we opened a different stream which      */
          /* extracted the interesting information out of this stream   */
          /* here.  That stream will still be open and the face will    */
          /* point to it).                                              */
          FT_Stream_Free( stream, external_stream );
          return error;
        }
      }

      if ( FT_ERR_NEQ( error, Unknown_File_Format ) )
        goto Fail2;
#endif  /* !FT_MACINTOSH && FT_CONFIG_OPTION_MAC_FONTS */

      /* no driver is able to handle this format */
      error = FT_THROW( Unknown_File_Format );

  Fail2:
      FT_Stream_Free( stream, external_stream );
      goto Fail;
    }

  Success:
    FT_TRACE4(( "FT_Open_Face: New face object, adding to list\n" ));

    /* add the face object to its driver's list */
    if ( FT_NEW( node ) )
      goto Fail;

    node->data = face;
    /* don't assume driver is the same as face->driver, so use */
    /* face->driver instead.                                   */
    FT_List_Add( &face->driver->faces_list, node );

    /* now allocate a glyph slot object for the face */
    FT_TRACE4(( "FT_Open_Face: Creating glyph slot\n" ));

    if ( face_index >= 0 )
    {
      error = FT_New_GlyphSlot( face, NULL );
      if ( error )
        goto Fail;

      /* finally, allocate a size object for the face */
      {
        FT_Size  size;


        FT_TRACE4(( "FT_Open_Face: Creating size object\n" ));

        error = FT_New_Size( face, &size );
        if ( error )
          goto Fail;

        face->size = size;
      }
    }

    /* some checks */

    if ( FT_IS_SCALABLE( face ) )
    {
      if ( face->height < 0 )
        face->height = (FT_Short)-face->height;

      if ( !FT_HAS_VERTICAL( face ) )
        face->max_advance_height = (FT_Short)face->height;
    }

    if ( FT_HAS_FIXED_SIZES( face ) )
    {
      FT_Int  i;


      for ( i = 0; i < face->num_fixed_sizes; i++ )
      {
        FT_Bitmap_Size*  bsize = face->available_sizes + i;


        if ( bsize->height < 0 )
          bsize->height = -bsize->height;
        if ( bsize->x_ppem < 0 )
          bsize->x_ppem = -bsize->x_ppem;
        if ( bsize->y_ppem < 0 )
          bsize->y_ppem = -bsize->y_ppem;

        /* check whether negation actually has worked */
        if ( bsize->height < 0 || bsize->x_ppem < 0 || bsize->y_ppem < 0 )
        {
          FT_TRACE0(( "FT_Open_Face:"
                      " Invalid bitmap dimensions for strike %d,"
                      " now disabled\n", i ));
          bsize->width  = 0;
          bsize->height = 0;
          bsize->size   = 0;
          bsize->x_ppem = 0;
          bsize->y_ppem = 0;
        }
      }
    }

    /* initialize internal face data */
    {
      FT_Face_Internal  internal = face->internal;


      internal->transform_matrix.xx = 0x10000L;
      internal->transform_matrix.xy = 0;
      internal->transform_matrix.yx = 0;
      internal->transform_matrix.yy = 0x10000L;

      internal->transform_delta.x = 0;
      internal->transform_delta.y = 0;

      internal->refcount = 1;

      internal->no_stem_darkening = -1;

#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING
      /* Per-face filtering can only be set up by FT_Face_Properties */
      internal->lcd_filter_func = NULL;
#endif
    }

    if ( aface )
      *aface = face;
    else
      FT_Done_Face( face );

    goto Exit;

  Fail:
    if ( node )
      FT_Done_Face( face );    /* face must be in the driver's list */
    else if ( face )
      destroy_face( memory, face, driver );

  Exit:
#ifdef FT_DEBUG_LEVEL_TRACE
    if ( !error && face_index < 0 )
    {
      FT_TRACE3(( "FT_Open_Face: The font has %ld face%s\n"
                  "              and %ld named instance%s for face %ld\n",
                  face->num_faces,
                  face->num_faces == 1 ? "" : "s",
                  face->style_flags >> 16,
                  ( face->style_flags >> 16 ) == 1 ? "" : "s",
                  -face_index - 1 ));
    }
#endif

    FT_TRACE4(( "FT_Open_Face: Return 0x%x\n", error ));

    return error;
  }

  
