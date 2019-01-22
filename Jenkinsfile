pipeline {
  agent {
    docker {
      image 'atf_slave:v.5'
    }

  }
  stages {
    stage('cmake') {
      steps {
        sh 'mkdir build && cd build && cmake ..'
      }
    }
    stage('build') {
      steps {
        sh '''cd build
&& make install -j4'''
      }
    }
    stage('archive') {
      steps {
        sh 'cd build && tar czf sdl.tar.gz bin'
      }
    }
  }
}