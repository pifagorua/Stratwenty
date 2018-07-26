#include <core.h>

Core::Core() :
    spawnid(0)
{
    selection_rect.x = 0;
    selection_rect.y = 0;

    selection_rect.w = 0;
    selection_rect.h = 0;
}

Core::~Core()
{
    cleanup();
}

void Core::cleanup()
{
    Logger::log("Core", "Started cleanup...");

    m_audiomgr.clear();

    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetWindowGrab(m_window, SDL_FALSE);

    if(m_iout != nullptr)
        SDL_DestroyRenderer(m_iout);
    if(m_window != nullptr)
        SDL_DestroyWindow(m_window);

    SDL_GL_DeleteContext(m_glcontext);

    SteamAPI_Shutdown();
    IMG_Quit();
    SDL_Quit();
}

void Core::init()
{
    //Config::loadCfg(m_appconf);
    pname = m_appconf.playername;
#ifdef STEAM
    if(!SteamAPI_IsSteamRunning())
        Logger::warn("Core", "SteamAPI init error!");
    else
        SteamAPI_Init();

    pname = unistring( SteamFriends()->GetPersonaName() );
    Logger::info("SteamAPI info", "Hello, "+pname+" :D !" );
#endif
    //

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
    {
        Logger::err("Core", SDL_GetError());
    }
    Logger::log("Core", "SDL2 init complete.");

#ifdef TESTING
    m_window    = SDL_CreateWindow("Stratwenty " DW_VERSION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DW_WIDTH, DW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    m_glcontext = SDL_GL_CreateContext(m_window);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetSwapInterval(1); // VSync

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
        Logger::err("Core", "Failed to init GLEW!");
#endif
    m_iout      = SDL_CreateRenderer(m_window, -1, 0);
#else
    m_window    = SDL_CreateWindow("Stratwenty " DW_VERSION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DW_WIDTH, DW_HEIGHT, SDL_WINDOW_SHOWN);

    m_iout      = SDL_CreateRenderer(m_window, 0, SDL_RENDERER_ACCELERATED);

    if(m_iout == nullptr)
    {
        Config::cfgwarn("HW renderer failed to init.\nRunning in SW mode.");
        m_iout = SDL_CreateRenderer(m_window, 0, SDL_RENDERER_SOFTWARE);
        if(m_iout == nullptr)
        {
            Config::cfgwarn("SW renderer failed to init.\nExiting with error: "+unistring(SDL_GetError()));
            exit(-1);
        }
    }
#endif

    if(m_window == nullptr)
    {
        Logger::err("Core", "SDL2 failed to create window.");
    }
//    Logger::log("Core", "Window & renderer created.");
//    SDL_SetWindowSize(m_window, m_appconf.app_width, m_appconf.app_height);
//    Logger::log("Core", "Window size set.");
//    SDL_SetWindowTitle(m_window, unistring(m_appconf.app_name+" " DW_VERSION).c_str() );
//    Logger::log("Core", "Window title set.");


//    map_rect.w = 164;
//    map_rect.h = 164;
//    map_rect.x = 2;
//    map_rect.y = m_appconf.app_height-map_rect.h-2;

//    menu_rect.w = m_appconf.app_width;
//    menu_rect.h = map_rect.h+4;
//    menu_rect.x = 0;
//    menu_rect.y = m_appconf.app_height-menu_rect.h;

//    SDL_ShowCursor(SDL_DISABLE);

//    m_audiomgr.init();
//    m_audiomgr.loadSounds(m_appconf);
//    m_audiomgr.loadMusic(m_appconf);

    //
    mouse_rect.w = 32;
    mouse_rect.h = 32;


    initGL();
}

int Core::exec()
{
    m_quit = false;

    while(!m_quit)
    {
        // =====================================================
        int c;
        while(SDL_PollEvent(&m_event))
        {
            switch (m_event.type) {
                case SDL_QUIT:
                    m_quit = true;
                break;
                case SDL_MOUSEBUTTONDOWN:
                    if(m_event.button.button > 0)
                    {
                        if(m_event.button.button == SDL_BUTTON_MIDDLE)
                            SDL_SetWindowGrab(m_window, SDL_TRUE);

                        m_processor.button_clicked[m_event.button.button-1] = true;
                        m_processor.button_down[m_event.button.button-1] = true;
                    }
                break;
                case SDL_MOUSEBUTTONUP:
                    if(m_event.button.button > 0)
                    {
                        //
                        if(m_event.button.button == SDL_BUTTON_MIDDLE)
                            SDL_SetWindowGrab(m_window, SDL_FALSE);
                        //
                        m_processor.button_down[m_event.button.button-1] = false;
                    }
                break;
                case SDL_MOUSEWHEEL:
                    if(m_event.wheel.y < 0)
                        m_camera.move(0.f, +1.0f, 0.f);
                    else if(m_event.wheel.y > 0)
                        m_camera.move(0.f, -1.0f, 0.f);
                break;
                case SDL_MOUSEMOTION:
                    if(m_processor.isMouseDown(SDL_BUTTON_MIDDLE))
                    {
                        if(m_event.motion.xrel > 0)
                            m_camera.rotate(glm::vec3(0,1,0), +10.f);
                        else if(m_event.motion.xrel < 0)
                            m_camera.rotate(glm::vec3(0,1,0), -10.f);
                    }
                break;
                default:
                break;
            }
        }
        // =====================================================
        processEvents();
        draw_objs3D();
    }
    return 0;
}

void Core::draw_objs3D()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    test_obj.draw(m_shadeprog, m_camera.matrix());

    SDL_GL_SwapWindow(m_window);

    SDL_Delay(1000/TARGET_FPS);
}

void Core::initGL()
{
    m_camera = Camera(90.f, (float)m_appconf.app_width/(float)m_appconf.app_height);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_MULTISAMPLE);

    initShaders(RES_ROOT "shaders/main.vert", RES_ROOT "shaders/main.frag"); // Vertex & Fragment shaders

    m_loader3d.LoadModel(RES_ROOT "test.obj", test_obj);
}

void Core::initShaders(unistring fvertex, unistring ffragment)
{
    unistring tvs, tfs; // TemporaryString (Vertex, Fragment)

    tvs.clear();
    ifstream vshader(fvertex);
    getline(vshader, tvs, (char)vshader.eof());
    vshader.close();

    tfs.clear();
    ifstream fshader(ffragment);
    getline(fshader, tfs, (char)fshader.eof());
    fshader.close();

    m_vsh = glCreateShader(GL_VERTEX_SHADER);
    m_fsh = glCreateShader(GL_FRAGMENT_SHADER);

    //

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Компилируем Вершинный шейдер
    printf("Compiling shader: %s\n", fvertex.c_str());
    char const * vsh_src = tvs.c_str();
    glShaderSource(m_vsh, 1, &vsh_src, NULL);
    glCompileShader(m_vsh);


    // Выполняем проверку Вершинного шейдера
    glGetShaderiv(m_vsh, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(m_vsh, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(m_vsh, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    }


    // Компилируем Фрагментный шейдер
    printf("Compiling shader: %s\n", ffragment.c_str());
    char const * fsh_src = tfs.c_str();
    glShaderSource(m_fsh, 1, &fsh_src, NULL);
    glCompileShader(m_fsh);


    // Проверяем Фрагментный шейдер
    glGetShaderiv(m_fsh, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(m_fsh, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(m_fsh, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    }


    // Создаем шейдерную программу и привязываем шейдеры к ней
    fprintf(stdout, "Connecting shaders...\n");
    m_shadeprog = glCreateProgram();
    glAttachShader(m_shadeprog, m_vsh);
    glAttachShader(m_shadeprog, m_fsh);
    glLinkProgram(m_shadeprog);


    // Проверяем шейдерную программу
    glGetProgramiv(m_shadeprog, GL_LINK_STATUS, &Result);
    glGetProgramiv(m_shadeprog, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(m_shadeprog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    }

    fflush(stdout);
    //
    glDeleteShader(m_vsh);
    glDeleteShader(m_fsh);
}

void Core::processEvents()
{
    glm::vec3 dir;
    if(m_processor.keyDown(SDL_SCANCODE_LEFT))
        dir.x = -1.f;
    else if(m_processor.keyDown(SDL_SCANCODE_RIGHT))
        dir.x = +1.f;

    if(m_processor.keyDown(SDL_SCANCODE_UP))
        dir.z = +1.f;
    else if(m_processor.keyDown(SDL_SCANCODE_DOWN))
        dir.z = -1.f;

    if(dir.length() != 0)
        m_camera.rtsmove(dir);


    if(m_processor.isMouseClicked(SDL_BUTTON_LEFT))
            m_processor.button_clicked[0] = false;
    if(m_processor.isMouseClicked(SDL_BUTTON_RIGHT))
        m_processor.button_clicked[2] = false;
}
